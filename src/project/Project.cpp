/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include <chrono>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>

#include "../config/GConf.h"
#include "../helpers.h"
#include "Project.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

/**
 * Project implementation
 */

Project::Project(const std::filesystem::path &root)
    : root_dir(root), build_dir(root / BUILD_DIR), makefile_(build_dir / MAKEFILE), reg_(Registry::get()),
      if_(Interface::get())
{
}

void Project::build(const bool release) const
{
  if (release)
    install_dependencies();

  if (!fs::exists(makefile_))
    generate_Makefile();

  const std::string dry_cmd = "make --dry-run -C " + build_dir.string();
  std::string dry_out = exec_command(dry_cmd);

  int compile_count = 0;
  std::istringstream iss(dry_out);
  std::string line;
  while (std::getline(iss, line))
  {
    if (line.find("echo \"[C] ") != std::string::npos || line.find("echo \"[CXX] ") != std::string::npos)
      compile_count++;
  }

  if (compile_count == 0)
  {
    if_.info("Everything is up to date, nothing to compile.");
    return;
  }

  if_.info("Building " + pconf.name + " (" + std::to_string(compile_count) + " files to compile)...");

  const std::string make_cmd = "make -C " + build_dir.string();

  if (const int result = std::system(make_cmd.c_str()); result == 0)
    if_.success("Project was successfully built in " + build_dir.string());
  else
    throw ZCException(ZCE_COMPILATION_ERROR, "Build failed with exit code " + std::to_string(result));
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
    throw ZCException(
        ZCE_MISSING_PROPERTY, "Authentication error: token is empty. Please run 'zc login' first."
    );

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
  string release_api_url =
      "https://api.github.com/repos/" + pconf.author + "/" + pconf.name + "/releases/tags/" + tag;

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

  const Dependency d{.name = pkg.name, .static_link = false, .version = *ranges::max_element(pkg.versions)};

  pconf.add_dependency(d);
  generate_compile_commands(); // for the LSPs
}

void Project::remove_dependency(const string &name)
{
  pconf.remove_dependency(name);
  generate_compile_commands(); // for the LSPs
}

void Project::install_dependencies() const
{
  const auto &net = Network::get();
  const json index = net.get_index();

  for (const auto &dependency : pconf.dependencies)
    reg_.install_from_server(dependency.name, dependency.version.string(), index);
}

void Project::generate_build_config() const
{
  generate_Makefile();
  generate_compile_commands();
}

void Project::generate_Makefile() const
{
  if (pconf.type == HEADER)
    return;

  GConf &gc(GConf::get());
  fs::create_directories(build_dir);
  std::vector<std::string> cxx_files;
  std::vector<std::string> c_files;

  get_sources(c_files, cxx_files);

  std::ostringstream mk;
  const fs::path cache_dir = get_zc_root() / CACHE_DIR;

  auto now_sec = chrono::floor<chrono::seconds>(chrono::system_clock::now());
  const string s = std::format("{:%F %T}", now_sec);
  mk << "# --- This file was automatically generated by ZC\n";
  mk << "# --- Date of creation: " << s << " (UTC)\n";
  mk << "# --- Do not edit this file manually !\n\n";

  mk << "CC = " << gc.c_compiler << "\n";
  mk << "CXX = " << gc.cxx_compiler << "\n";

  mk << "CFLAGS = -std=" << gc.c_std << " -MMD -MP";
  if (pconf.type == LIB)
    mk << " -fPIC";
  for (const auto &flag : pconf.flags) mk << " " << flag;
  for (const auto &dep : pconf.dependencies)
    mk << " -I"
       << (cache_dir / dep.name / dep.version.string() / INCLUDE_DIR).string(); // TODO : handle std libraries
  for (const auto &inc : pconf.include_dirs)
    if (fs::exists(root_dir / inc))
      mk << " -I../" << inc;
  mk << "\n";

  mk << "CXXFLAGS = -std=" << gc.cxx_std << " -MMD -MP";
  if (pconf.type == LIB)
    mk << " -fPIC";
  for (const auto &flag : pconf.flags) mk << " " << flag;
  for (const auto &dep : pconf.dependencies)
    mk << " -I"
       << (cache_dir / dep.name / dep.version.string() / INCLUDE_DIR).string(); // TODO : handle std libraries
  for (const auto &inc : pconf.include_dirs)
    if (fs::exists(root_dir / inc))
      mk << " -I../" << inc;
  mk << "\n";

  mk << "COBJS = ";
  for (const auto &file : c_files) mk << file << ".o ";
  mk << "\n";

  mk << "CXXOBJS = ";
  for (const auto &file : cxx_files) mk << file << ".o ";
  mk << "\n\n";

  switch (pconf.type)
  {
  case BIN:
    Makefile_bin(mk);
    break;
  case LIB:
    Makefile_lib(mk);
    break;
  case COMPOSE:
    Makefile_compose(mk);
    break;
  default:
    break;
  }

  mk << "%.c.o: ../%\n";
  mk << "\t@mkdir -p $(dir $@)\n";
  mk << "\t@echo \"[C]    $<\"\n";
  mk << "\t@$(CC) $(CFLAGS) -c $< -o $@\n\n";

  mk << "%.cpp.o: ../%\n";
  mk << "\t@mkdir -p $(dir $@)\n";
  mk << "\t@echo \"[CXX]  $<\"\n";
  mk << "\t@$(CXX) $(CXXFLAGS) -c $< -o $@\n\n";

  mk << "-include $(COBJS:.o=.d)\n";
  mk << "-include $(CXXOBJS:.o=.d)\n";

  std::ofstream out(makefile_);
  out << mk.str();
}

void Project::Makefile_bin(std::ostringstream &mk) const
{
  mk << "TARGET = " << pconf.name << "\n\n";
  mk << "all: $(TARGET)\n\n";
  mk << "$(TARGET): $(COBJS) $(CXXOBJS)\n";
  mk << "\t@echo \"[Link] Linking executable $@\"\n";
  mk << "\t@$(CXX) $(CXXFLAGS) -o $@ $^\n\n";
}

void Project::Makefile_lib(std::ostringstream &mk) const
{
  mk << "TARGET_STATIC = lib" << pconf.name << ".a\n"; // TODO : handle other operating systems
  mk << "TARGET_SHARED = lib" << pconf.name << ".so\n\n";
  mk << "all: $(TARGET_STATIC) $(TARGET_SHARED)\n\n";

  mk << "$(TARGET_STATIC): $(COBJS) $(CXXOBJS)\n";
  mk << "\t@echo \"[AR]   Archiving static library $@\"\n";
  mk << "\t@ar rcs $@ $^\n\n";

  mk << "$(TARGET_SHARED): $(COBJS) $(CXXOBJS)\n";
  mk << "\t@echo \"[Link] Linking shared library $@\"\n";
  mk << "\t@$(CXX) -shared -o $@ $^\n\n";
}

void Project::Makefile_compose(std::ostringstream &mk) const
{
  mk << "all:\n";
  mk << "\t@echo \"Compose project type is not yet fully implemented\"\n\n";
}

void Project::generate_compile_commands() const
{
  fs::create_directories(build_dir);
}

void Project::get_sources(std::vector<std::string> &c_files, std::vector<std::string> &cxx_files) const
{
  bool found_any_src = false;
  for (const auto &src_dir : pconf.src_dirs)
  {
    fs::path full_src_dir = root_dir / src_dir;
    if (!fs::exists(full_src_dir))
      continue;

    found_any_src = true;
    for (const auto &entry : fs::recursive_directory_iterator(full_src_dir))
    {
      if (string ext = entry.path().extension(); entry.is_regular_file() && !ext.empty())
      {
        ext.erase(0, 1);
        if (is_c(ext))
          c_files.push_back(fs::relative(entry.path(), root_dir).string());
        else if (is_cxx(ext))
          cxx_files.push_back(fs::relative(entry.path(), root_dir).string());
      }
    }
  }

  if (!found_any_src)
    throw ZCException(ZCE_NO_SOURCE_FILES, "None of the source directories exist");
}

} // namespace zc
