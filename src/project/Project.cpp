/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include <nlohmann/json.hpp>

#include "Project.h"
#include "../config/GConf.h"
#include "../helpers.h"

ZC_DEV_CONFIG_JSON

/**
 * Project implementation
 */

Project::Project(const std::filesystem::path &root) : root_dir(root), build_dir(root / BUILD_DIR), reg_(Registry::get()), if_(Interface::get())
{
}

void Project::build(bool release, const bool force)
{
  if (fs::exists(build_dir) && force)
    clean();


}

void Project::clean() const
{
  if (fs::exists(build_dir) && fs::is_directory(build_dir))
    if (fs::remove_all(build_dir) > 0)
      if_.info("Cleaned " + build_dir.string());
}

void Project::publish()
{
  if_.info("Preparing to publish package " + pconf.name + " at version " + pconf.version.string() + "...");

  // Version is already in its constructor
  if (pconf.author.empty())
    throw ZCException(ZCE_MISSING_PROPERTY, "Package author is missing in zc.json");
  if (pconf.name.empty())
    throw ZCException(ZCE_MISSING_PROPERTY, "Package name is missing in zc.json");

  GConf &gc(GConf::get());
  if (gc.token.empty())
    throw ZCException(ZCE_MISSING_PROPERTY, "Authentication error: token is empty. Please run 'zc login' first.");

  if_.info("Verifying GitHub identity...");
  Network &net(Network::get());
  string user_info_raw = net.get("https://api.github.com/user", "", gc.token); // empty payload
  auto user_info = nlohmann::json::parse(user_info_raw);
  string github_login = user_info["login"];

  if (github_login != pconf.author)
    throw ZCException(
      ZCE_AUTHENTICATION_ERROR, "Identity mismatch: You are logged in as '" + github_login +
                                    "' but the author in zc.json is '" + pconf.author +
                                    "'. Publication blocked for security reasons."
    );

  if_.success("Authenticated as " + github_login);

  string tag = "v" + pconf.version.string();
  string archive_url =
      "https://github.com/" + pconf.author + "/" + pconf.name + "/archive/refs/tags/" + tag + ".tar.gz";

  if_.info("Expected archive URL: " + archive_url);

  const fs::path tmp_dir = get_zc_root() / TMP_DIR;
  const fs::path archive_path = tmp_dir / (pconf.name + "_" + tag + ".tar.gz");
  fs::create_directories(tmp_dir);

  // Ensure the release exists on GitHub
  string release_api_url = "https://api.github.com/repos/" + pconf.author + "/" + pconf.name + "/releases/tags/" + tag;

  try
  {
    if_.info("Checking if release '" + tag + "' exists on GitHub...");
    net.get(release_api_url, "", gc.token);
    if_.success("Release " + tag + " found!");
  }
  catch (const ZCException &)
  {
    if_.warning("Release " + tag + " not found. Attempting to create it...");

    if (!if_.ask("Do you want to create tag " + tag + "?"))
      return;

    nlohmann::json release_payload;
    release_payload["tag_name"] = tag;
    release_payload["name"] = "Release " + tag;
    release_payload["body"] = "Automated release by ZC build tool.";
    release_payload["generate_release_notes"] = true;

    string create_release_url =
        "https://api.github.com/repos/" + pconf.author + "/" + pconf.name + "/releases";

    try
    {
      net.post(create_release_url, release_payload.dump(), gc.token);
      if_.success("Release " + tag + " created successfully!");
    }
    catch (const ZCException &) // TODO : handle different error types directly in Network.cpp
    {
      throw ZCException(
          ZCE_NETWORK_ERROR, "Failed to create release. Ensure the tag exists or you have enough permissions."
      );
    }
  }

  // Download and Hash
  if_.info("Downloading archive for verification...");
  net.download(archive_url, archive_path);

  string sha = sha256(archive_path);
  if_.success("SHA-256 calculated: " + sha);
  fs::remove(archive_path);

  // Upload Recipe to Registry
  nlohmann::json recipe;
  recipe["name"] = pconf.name;
  recipe["version"] = pconf.version.string();
  recipe["url"] = archive_url;
  recipe["sha256"] = sha;
  recipe["owner"] = pconf.author;

  string file_path = "packages/" + pconf.name + "/" + pconf.version.string() + ".json";
  nlohmann::json payload;
  payload["message"] = "Publish " + pconf.name + " v" + pconf.version.string();
  payload["content"] = base64_encode(recipe.dump(2));

  string api_url = "https://api.github.com/repos/" GH_REPO "/contents/" + file_path;

  if_.info("Uploading recipe to registry...");
  try
  {
    // Safety Check: Ensure the package owner doesn't change
    string package_dir_url = "https://api.github.com/repos/" GH_REPO "/contents/packages/" + pconf.name;
    try
    {
      if_.debug("Verifying package ownership in registry...");
      string dir_content_raw = net.get(package_dir_url, "", gc.token);

      if (auto dir_content = nlohmann::json::parse(dir_content_raw);
          dir_content.is_array() && !dir_content.empty())
      {
        // Get the first file found to check the original owner
        string first_version_url = dir_content[0]["download_url"];
        string first_version_raw = net.get(first_version_url, "", gc.token);

        if (auto first_version_json = nlohmann::json::parse(first_version_raw);
            first_version_json.contains("owner") && first_version_json["owner"] != pconf.author)
        {
          throw ZCException(
              ZCE_AUTHENTICATION_ERROR, "Security Violation: This package is owned by '" +
                                           first_version_json["owner"].get<string>() +
                                           "'. You cannot publish a new version."
          );
        }
      }
    }
    catch (const ZCException &e)
    {
      // 404 is fine, it means it's a new package
      if (string(e.what()).find("404") == string::npos)
        throw;
    }

    // Upload the recipe
    net.put(api_url, payload.dump(), gc.token);
    if_.success("Your package has been successfully published to the registry!");
  }
  catch (const ZCException &e)
  {
    if (string(e.what()).find("422") != string::npos)
    {
      throw ZCException(
          ZCE_VERSION_ALREADY_EXISTS, "Version " + pconf.version.string() + " already exists in the registry."
      );
    }
    throw ZCException(ZCE_NETWORK_ERROR, "Failed to upload recipe: " + string(e.what()));
  }
}

void Project::add_dependency(const string &name)
{
  RegistryPkg pkg = reg_.get_pkg(name); // throws an error if package is not found

  const Dependency d{
    .name = pkg.name,
    .static_link = false,
    .version = *ranges::max_element(pkg.versions)
  };

  pconf.add_dependency(d);
}

void Project::remove_dependency(const string &name)
{
  pconf.remove_dependency(name);
}

/**
 * @return void
 */
void Project::generate_Makefile()
{
}

/**
 * @return void
 */
void Project::generate_compile_commands()
{
}

void Project::install_dependencies() const
{
  const auto &net = Network::get();
  const json index = net.get_index();

  for (const auto &dependency: pconf.dependencies)
    reg_.install_from_server(dependency.name, dependency.version.string(), index);
}
