#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <commands/Install.hh>
#include <helpers.hh>
#include <interface.hh>
#include <nlohmann/json.hpp>
#include <objects/Registry.hh>
#include <objects/ZCError.hh>

using namespace std;
namespace fs = std::filesystem;

Install::Install(
    const std::vector<std::string> &targets, const std::string &path, const bool global, const bool force,
    const bool quiet
)
    : Command(force, quiet), path_(path), registry_(Registry(global))
{
  if (!global && !path_.empty() && getProjectRoot(path_) == getProjectRoot())
    throw ZCError(ZC_BAD_COMMAND, "Cannot install library as its own dependency");

  for (const auto &target : targets)
  {
    string requested_version = "";
    size_t at_pos = target.find('@');

    if (at_pos != string::npos)
      targets_.push_back({target.substr(0, at_pos), target.substr(at_pos + 1)});
    else
      targets_.push_back({target, ""});
  }
}

int Install::operator()()
{
  if (!targets_.empty() && !path_.empty())
    throw ZCError(ZC_INCOMPATIBLE_FLAGS, "Cannot install from remote and from local path at the same time");

  else if (path_.empty() && !targets_.empty())
    installFromServer(); // Only path is empty : install targets from server

  else if (targets_.empty() && !path_.empty())
    installFromPath(); // Only targets is empty : install from path

  else
    installFromJson(); // Both path and targets empty : install from registry.json

  registry_.write();
  return 0;
}

void Install::installFromJson()
{
  // Just fill targets and then call installFromServer()
  const std::vector<Package> pkgs = registry_.getPackages();
  for (const auto &p : pkgs)
    if (p.origin_ != "std")
      targets_.push_back({p.name_, p.version_});
  installFromServer();
}

void Install::installFromServer()
{
  const fs::path tmp_dir = getZCRootDir() / TMP_DIR;

  log_info("Fetching registry index from " REGISTRY_URL "...");

  fs::create_directories(tmp_dir);
  const fs::path index_path = tmp_dir / "index.json";
  string curl_cmd = "curl -sfL " REGISTRY_URL " -o " + index_path.string();

  if (system(curl_cmd.c_str()) != 0)
    throw ZCError(ZC_INTERNAL_ERROR, "Network error: Failed to download registry index.");

  std::ifstream index_file(index_path);
  nlohmann::json index_json;
  try
  {
    index_file >> index_json;
  }
  catch (const nlohmann::json::parse_error &e)
  {
    throw ZCError(ZC_CONFIG_PARSING_ERROR, "Invalid JSON received from registry server.");
  }

  for (const auto &[name, version] : targets_)
  {
    if (!index_json.contains("packages") || !index_json["packages"].contains(name))
    {
      log_warning("Package '" + name + "' not found in the remote registry.");
      continue;
    }

    string version_to_install = version;
    if (version.empty())
      version_to_install = index_json["packages"][name]["latest"];

    if (!index_json["packages"][name]["versions"].contains(version_to_install))
    {
      throw ZCError(
          ZC_BAD_COMMAND, "Version " + version_to_install + " of package '" + name + "' does not exist."
      );
    }

    string download_url = index_json["packages"][name]["versions"][version_to_install].value("url", "");
    if (download_url.empty())
      throw ZCError(ZC_CONFIG_MISSING_PROPERTY, "Missing 'url' for package " + name);

    log_info("Downloading " + name + " v" + version_to_install + "...");

    const fs::path archive_path = tmp_dir / (name + ".tar.gz");
    const fs::path extract_path = tmp_dir / name;

    // Download archive
    string download_cmd = "curl -sfL " + download_url + " -o " + escape_shell_arg(archive_path.string());
    if (system(download_cmd.c_str()) != 0)
      throw ZCError(ZC_INTERNAL_ERROR, "Network error: Failed to download archive for " + name);

    // Check hash
    string expected_hash = index_json["packages"][name]["versions"][version_to_install]["sha256"];

    string actual_hash = execAndGetOutput(
        ("shasum -a 256 " + escape_shell_arg(archive_path.string()) + " | awk '{ print $1 }'").c_str()
    );
    actual_hash.erase(actual_hash.find_last_not_of(" \n\r\t") + 1);

    if (expected_hash != "SKIP" && actual_hash != expected_hash)
    {
      fs::remove(archive_path);
      throw ZCError(
          ZC_INTERNAL_ERROR, "SECURITY ALERT: The SHA-256 hash of " + name + " does not match the registry!"
      );
    }
    else
      log_success("Hash is correct");

    // Extract archive
    fs::create_directories(extract_path);
    string tar_cmd = "tar -xzf " + escape_shell_arg(archive_path.string()) + " -C " +
                     escape_shell_arg(extract_path.string());
    if (system(tar_cmd.c_str()) != 0)
      throw ZCError(ZC_INTERNAL_ERROR, "Failed to extract package " + name);

    // Install package
    fs::path project_root = extract_path;
    for (const auto &entry : fs::directory_iterator(extract_path))
    {
      if (entry.is_directory())
      {
        project_root = entry.path();
        break;
      }
    }
    // TODO : change main to indicate on which server it was found
    registry_.installPackage(project_root, force_, quiet_, "main");
  }

  fs::remove_all(tmp_dir);
}

void Install::installFromPath()
{
  registry_.installPackage(getProjectRoot(path_), force_, quiet_, "local");
}
