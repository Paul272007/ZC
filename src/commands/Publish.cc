#include <string>

#include <commands/Publish.hh>
#include <helpers.hh>
#include <interface.hh>
#include <nlohmann/json.hpp>
#include <objects/ProjectSettings.hh>
#include <objects/Registry.hh>
#include <objects/ZCError.hh>

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;

Publish::Publish(const bool force, const bool quiet)
    : Command(force, quiet), p_settings_(ProjectSettings(getProjectRoot()))
{
}

int Publish::operator()()
{
  log_info(
      "Preparing to publish package '" + p_settings_.name_ + "' v" + p_settings_.version_->string() + "..."
  );

  string archive_url = input("Enter the public URL of your .tar.gz archive (e.g., GitHub Release): ");

  const fs::path tmp_dir = getZCRootDir() / TMP_DIR;
  const fs::path archive_path = tmp_dir / "temp_publish.tar.gz";
  fs::create_directories(tmp_dir);

  log_info("Downloading archive to calculate SHA-256 hash...");

  // Download user archive
  string curl_cmd =
      "curl -sfL " + escape_shell_arg(archive_url) + " -o " + escape_shell_arg(archive_path.string());
  if (system(curl_cmd.c_str()) != 0)
    throw ZCError(ZC_INTERNAL_ERROR, "Failed to download the archive. Check the URL.");

  // Calculate archive hash
  string sha256 = "";
  try
  {
    string hash_cmd = "shasum -a 256 " + escape_shell_arg(archive_path.string()) + " | awk '{ print $1 }'";
    sha256 = execAndGetOutput(hash_cmd.c_str());

    sha256.erase(sha256.find_last_not_of(" \n\r\t") + 1);
  }
  catch (...)
  {
    throw ZCError(ZC_INTERNAL_ERROR, "Failed to calculate SHA-256 hash.");
  }

  fs::remove(archive_path);

  // Create recipe
  json recipe;
  recipe["name"] = p_settings_.name_;
  recipe["version"] = p_settings_.version_->string();
  recipe["url"] = archive_url;
  recipe["sha256"] = sha256;

  string file_path = "packages/" + p_settings_.name_ + "/" + p_settings_.version_->string() + ".json";

  string magic_link =
      "https://github.com/" GH_REPO "/new/main?filename=" + file_path + "&value=" + urlEncode(recipe.dump(2));

  if (!quiet_)
  {
    success("Hash verified: " + sha256);
    info("Your package is ready to be published!");
    info("To submit it to the global registry, simply click this link:");
    info(U_BLUE + magic_link + COLOR_RESET);
    info("GitHub will automatically format your Pull Request.");
    info("Just scroll down and click 'Propose new file'!");
  }

  return 0;
}
