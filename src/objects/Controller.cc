#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "files.hh"
#include "helpers.hh"
#include "interface.hh"
#include "nlohmann/json.hpp"
#include "objects/Controller.hh"
#include "objects/LocalController.hh"
#include "objects/ZCError.hh"

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

Controller::~Controller()
{
  delete c_;
  delete r_;
}

void Controller::installFromPath(const std::filesystem::path &root, bool quiet)
{
  LocalController pc(log_, force_, root);
  installPackageFromPath(pc, quiet, "local");
}

void Controller::clean()
{
  fs::remove_all(tmp_dir_);
}

void Controller::installFromJson(bool quiet)
{
  Visited v;
  installFromJson(quiet, v);
}

void Controller::installFromJson(bool quiet, Visited &visited)
{
  Targets targets;
  const std::vector<Package> pkgs = r_->getPackages();

  for (const auto &p : pkgs)
  {
    // On ne rajoute que ce qui n'est pas standard ET pas encore visité
    if (p.origin != "std" && visited.find(p.name) == visited.end())
    {
      targets.push_back({p.name, p.version.string()});
      visited.insert(p.name); // On le marque comme "à traiter / traité"
    }
  }

  if (!targets.empty())
  {
    installFromServer(targets, quiet, visited);
  }
}

void Controller::downloadIndex()
{
  log_(LogLevel::INFO, "Fetching registry index from " INDEX_URL "...");

  const fs::path index_path = tmp_dir_ / INDEX;
  string curl_cmd = "curl -sfL " INDEX_URL " -o " + index_path.string();

  if (system(curl_cmd.c_str()) != 0)
    throw ZCError(ZC_INTERNAL_ERROR, "Network error: Failed to download registry index.");
}

void Controller::downloadArchive(const std::string &url, const std::filesystem::path &path)
{
  string download_cmd = "curl -sfL " + url + " -o " + escape_shell_arg(path.string());
  if (system(download_cmd.c_str()) != 0)
    throw ZCError(ZC_INTERNAL_ERROR, "Network error: Failed to download archive " + url);
}

void Controller::extractAndInstall(const fs::path &archive, const fs::path &dest, bool quiet, Visited &v)
{
  fs::create_directories(dest);
  string tar_cmd =
      "tar -xzf " + escape_shell_arg(archive.string()) + " -C " + escape_shell_arg(dest.string());

  if (system(tar_cmd.c_str()) != 0)
    throw ZCError(ZC_INTERNAL_ERROR, "Extraction failed.");

  fs::path project_root = dest;
  for (const auto &entry : fs::directory_iterator(dest))
  {
    if (entry.is_directory())
    {
      project_root = entry.path();
      break;
    }
  }

  LocalController pc(log_, force_, project_root);
  pc.installFromJson(quiet, v);
  installPackageFromPath(pc, quiet, "main");
}

string Controller::resolvePackageUrl(const string &name, const string &version, const json &index)
{
  if (!index.contains("packages") || !index["packages"].contains(name))
    throw ZCError(ZC_BAD_COMMAND, "Package '" + name + "' not found.");

  string version_to_install =
      version.empty() ? index["packages"][name]["latest"].get<std::string>() : version;

  if (!index["packages"][name]["versions"].contains(version_to_install))
    throw ZCError(ZC_BAD_COMMAND, "Version " + version_to_install + " missing.");

  return index["packages"][name]["versions"][version_to_install].value("url", "");
}

void Controller::verifyPackageHash(const fs::path &archive, const string &expected)
{
  if (expected == "SKIP")
    return;

  string cmd = "shasum -a 256 " + escape_shell_arg(archive.string()) + " | awk '{ print $1 }'";
  string actual = execAndGetOutput(cmd.c_str());
  actual.erase(actual.find_last_not_of(" \n\r\t") + 1);

  if (actual != expected)
  {
    fs::remove(archive);
    throw ZCError(ZC_INTERNAL_ERROR, "SECURITY ALERT: Hash mismatch!");
  }
  log_(LogLevel::SUCCESS, "Hash is correct");
}

void Controller::processPackage(
    const string &name, const string &version, const json &index, bool quiet, Visited &v
)
{
  string url = resolvePackageUrl(name, version, index);
  log_(LogLevel::INFO, "Downloading " + name + "...");

  const fs::path archive_path = tmp_dir_ / (name + ".tar.gz");
  const fs::path extract_path = tmp_dir_ / name;

  downloadArchive(url, archive_path);

  string version_key = version.empty() ? index["packages"][name]["latest"].get<std::string>() : version;
  verifyPackageHash(archive_path, index["packages"][name]["versions"][version_key]["sha256"]);

  extractAndInstall(archive_path, extract_path, quiet, v);
}

void Controller::installFromServer(Targets &targets, bool quiet)
{
  Visited v;
  installFromServer(targets, quiet, v);
}

void Controller::installFromServer(Targets &targets, bool quiet, Visited &visited)
{
  fs::create_directories(tmp_dir_);
  downloadIndex();
  json index = parseJsonFile(tmp_dir_ / INDEX);

  for (const auto &[name, version] : targets)
  {
    try
    {
      processPackage(name, version, index, quiet, visited);
    }
    catch (const ZCError &e)
    {
      log_(LogLevel::ERROR, e.what());
    }
  }
  clean();
}

void Controller::installPackageFromPath(LocalController &pc, bool quiet, const std::string &origin)
{
  // Parameter = Controller for the project that is being installed
  if (r_->pkgExists(pc.lc_->name_) && !force_)
    if (!ask(
            "The package '" + pc.lc_->name_ + "' seems to be already installed. Do you want to reinstall it?"
        ))
      return;
  pc.buildProject(quiet);

#ifdef DEBUG_MODE
  log_(LogLevel::DEBUG, "Project compiled");
#endif

  bool is_bin = false;
  switch (pc.lc_->type_)
  {
  case Type::LIB:
    installLibrary(pc);
    is_bin = false;
    break;
  case Type::BIN:
    installExecutable(pc);
    is_bin = true;
    break;
  case Type::UNDEF:
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "The project type must be 'lib' or 'bin'");
  }

  r_->indexPackage(
      Package{
          .name = pc.lc_->name_,
          .binary = pc.lc_->target_,
          .origin = origin,
          .version = pc.lc_->version_.value(),
          .is_exec = is_bin
      }
  );
  log_(LogLevel::SUCCESS, "Package " + pc.lc_->name_ + " installed successfully.");
}

void Controller::installLibrary(LocalController &pc)
{
  const fs::path dest_include = include_dir_ / pc.lc_->name_;
  const fs::path dest_lib = lib_dir_ / pc.lc_->name_;

  pc.log_(LogLevel::INFO, "Installing headers...");

  fs::create_directories(dest_include);
  fs::copy(
      pc.lc_->include_folder_, dest_include,
      fs::copy_options::recursive | fs::copy_options::overwrite_existing
  );

  pc.log_(LogLevel::INFO, "Installing libraries...");

  fs::create_directories(dest_lib);

  for (const auto &entry : fs::recursive_directory_iterator(pc.root_dir_ / BUILD_DIR))
  {
    if (string filename = entry.path().filename().string();
        filename.find(pc.lc_->target_) != string::npos &&
        (entry.path().extension() == ".a" || entry.path().extension() == ".so" ||
         entry.path().extension() == ".dylib" || entry.path().extension() == ".lib"))
    {
      fs::copy_file(entry.path(), dest_lib / filename, fs::copy_options::overwrite_existing);
    }
  }
}

void Controller::installExecutable(LocalController &pc)
{
  const fs::path exe_dest = bin_dir_ / pc.lc_->target_;
  const fs::path exe_source = pc.root_dir_ / BUILD_DIR / pc.lc_->target_;

  pc.log_(LogLevel::INFO, "Installing global binary...");

  fs::create_directories(bin_dir_);

  if (!fs::exists(exe_source))
    throw ZCError(ZC_NOT_FOUND, "The compiled binary was not found: " + exe_source.string());

  fs::copy_file(exe_source, exe_dest, fs::copy_options::overwrite_existing);

  fs::permissions(
      exe_dest, fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec, fs::perm_options::add
  );
}

bool Controller::removePackage(const std::string &pkg_name)
{
  Package p;
  try
  {
    p = r_->unindexPackage(pkg_name);
  }
  catch (const ZCError &)
  {
    return false;
  }

  if (p.is_exec)
  {
    if (fs::exists(bin_dir_ / p.binary))
      fs::remove(bin_dir_ / p.binary);
  }
  else
  {
    if (fs::exists(include_dir_ / pkg_name))
      fs::remove_all(include_dir_ / pkg_name);

    if (fs::exists(lib_dir_ / pkg_name))
      fs::remove_all(lib_dir_ / pkg_name);
  }
  return true;
}

std::vector<std::pair<std::string, std::string>>
Controller::parsePackages(const std::vector<std::string> &targets)
{
  std::vector<std::pair<std::string, std::string>> results;
  for (const auto &target : targets)
  {
    string requested_version = "";
    size_t at_pos = target.find('@');

    if (at_pos != string::npos)
      results.push_back({target.substr(0, at_pos), target.substr(at_pos + 1)});
    else
      results.push_back({target, ""});
  }
  return results;
}

bool Controller::isInstalled(const std::string &pkg)
{
  return r_->pkgExists(pkg);
}
