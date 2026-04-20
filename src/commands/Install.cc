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
    : Command(force, quiet), targets_(targets), path_(path), registry_(Registry(global))
{
  if (!global && !path_.empty() && getProjectRoot(path_) == getProjectRoot())
    throw ZCError(ZC_BAD_COMMAND, "Cannot install library as its own dependency");
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
      targets_.push_back(p.name_);
  installFromServer();
}

void Install::installFromServer()
{
  const fs::path tmp_dir = getZCRootDir() / TMP_DIR;

  if (!quiet_)
    info("Fetching registry index from " REGISTRY_URL "...");

  fs::create_directories(tmp_dir);
  const fs::path index_path = tmp_dir / "index.json";
  string curl_cmd = "curl -sL " REGISTRY_URL " -o " + index_path.string();

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

  for (const string &target : targets_)
  {
    if (!index_json.contains("packages") || !index_json["packages"].contains(target))
    {
      if (!quiet_)
        warning("Package '" + target + "' not found in the remote registry.");
      continue;
    }

    string latest_version = index_json["packages"][target]["latest"];
    string download_url = index_json["packages"][target]["versions"][latest_version]["url"];

    if (!quiet_)
      info("Downloading " + target + " v" + latest_version + "...");

    const fs::path archive_path = tmp_dir / (target + ".tar.gz");
    const fs::path extract_path = tmp_dir / target;

    string download_cmd = "curl -sL " + download_url + " -o " + archive_path.string();
    if (system(download_cmd.c_str()) != 0)
      throw ZCError(ZC_INTERNAL_ERROR, "Network error: Failed to download archive for " + target);

    fs::create_directories(extract_path);
    string tar_cmd = "tar -xzf " + archive_path.string() + " -C " + extract_path.string();
    if (system(tar_cmd.c_str()) != 0)
      throw ZCError(ZC_INTERNAL_ERROR, "Failed to extract package " + target);

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
