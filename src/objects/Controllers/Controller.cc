#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "helpers.hh"
#include "interface.hh"
#include "nlohmann/json.hpp"
#include "objects/Controllers/Controller.hh"
#include "objects/Controllers/LocalController.hh"
#include "objects/Registries/Registry.hh"
#include "objects/Version.hh"
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
  buildAndIndex(pc, quiet, "local", false);
}

void Controller::updateFromPath(const std::filesystem::path &root, bool quiet)
{
  LocalController pc(log_, force_, root);
  // Throws error is package is not found
  Package p = r_->getPackage(pc.lc_->name_);
  if (p.version >= pc.lc_->version_ && !force_)
    throw ZCError(
        ZC_BAD_COMMAND, "The version of the installed package is equal or higher. Use --force to override."
    );

  buildAndIndex(pc, quiet, "local", true);
}

void Controller::clean() const
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
    if (p.origin != "std" && p.origin != "local" && visited.find(p.name) == visited.end())
    {
      if (isInstalled(p.name))
      {
        log_(LogLevel::INFO, "Package " + p.name + " already installed");
        continue;
      }
      targets.push_back({p.name, p.version.string()});
      visited.insert(p.name);
    }
  }

  if (!targets.empty())
  {
    installFromServer(targets, quiet, visited);
  }
}

void Controller::updateFromJson(bool quiet)
{
  Visited v;
  updateFromJson(quiet, v);
}

void Controller::updateFromJson(bool quiet, Visited &visited)
{
  json index = net_.getIndex(log_);
  Targets targets;
  const std::vector<Package> pkgs = r_->getPackages();

  for (const auto &p : pkgs)
  {
    if (p.origin != "std" && p.origin != "local" && visited.find(p.name) == visited.end())
    {
      Version latest(index["packages"][p.name]["latest"].get<std::string>());
      if (latest == p.version)
      {
        log_(LogLevel::INFO, "Package " + p.name + " is already installed at latest version.");
      }
      else
      {
        targets.emplace_back(p.name, p.version.string());
        visited.insert(p.name);
      }
    }
  }

  if (!targets.empty())
  {
    updateFromServer(targets, quiet, index, visited);
  }
}

void Controller::updateFromServer(Targets &targets, bool quiet)
{
  Visited visited;
  json index = net_.getIndex(log_);
  updateFromServer(targets, quiet, index, visited);
}

void Controller::updateFromServer(Targets &targets, bool quiet, nlohmann::json &index, Visited &visited)
{
  for (const auto &[name, version] : targets)
  {
    try
    {
      if (!isInstalled(name))
        throw ZCError(ZC_PACKAGE_NOT_FOUND, "Package " + name + " is not installed");

      updatePackageFromServer(name, version, index, quiet, visited);
    }
    catch (const ZCError &e)
    {
      log_(LogLevel::ERROR, e.what());
    }
  }
  clean();
}

void Controller::updatePackageFromServer(
    const std::string &name, const std::string &version, const nlohmann::json &index, bool quiet, Visited &v
)
{
  if (!index.contains("packages") || !index["packages"].contains(name))
    throw ZCError(ZC_PACKAGE_NOT_FOUND, "Package '" + name + "' not found.");

  string target_version = version.empty() ? index["packages"][name]["latest"].get<std::string>() : version;

  if (isInstalled(name))
  {
    Package installed_pkg = r_->getPackage(name);
    if (installed_pkg.version.string() == target_version && !force_)
    {
      log_(LogLevel::INFO, "Skipped package " + name + " (already up-to-date at v" + target_version + ")");
      return;
    }
  }

  if (!index["packages"][name]["versions"].contains(target_version))
    throw ZCError(ZC_BAD_PACKAGE_DECLARATION, "Version " + target_version + " missing from remote registry.");

  string url = index["packages"][name]["versions"][target_version].value("url", "");

  log_(LogLevel::INFO, "Downloading " + url + "...");
  const fs::path archive_path = tmp_dir_ / (name + ".tar.gz");
  const fs::path extract_path = tmp_dir_ / name;
  net_.download(url, archive_path);

  verifyPackageHash(
      archive_path, index["packages"][name]["versions"][target_version].value("sha256", "SKIP")
  );

  extractAndInstall(archive_path, extract_path, quiet, v, true);
}

void Controller::extractAndInstall(
    const fs::path &archive, const fs::path &dest, bool quiet, Visited &v, bool isUpdate
)
{
  fs::create_directories(dest);

  log_(LogLevel::INFO, "Extracting archive...");
  if (!extract_archive(archive, dest))
    throw ZCError(ZC_TAR_ERROR, "Extraction failed.");

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
  buildAndIndex(pc, quiet, "main", isUpdate);
}

string Controller::resolvePackageUrl(const string &name, const string &version, const json &index)
{
  if (!index.contains("packages") || !index["packages"].contains(name))
    throw ZCError(ZC_PACKAGE_NOT_FOUND, "Package '" + name + "' not found.");

  string version_to_install =
      version.empty() ? index["packages"][name]["latest"].get<std::string>() : version;

  if (!index["packages"][name]["versions"].contains(version_to_install))
    throw ZCError(ZC_BAD_PACKAGE_DECLARATION, "Version " + version_to_install + " missing.");

  return index["packages"][name]["versions"][version_to_install].value("url", "");
}

void Controller::verifyPackageHash(const fs::path &archive, const string &expected)
{
  if (expected == "SKIP")
    return;

  try
  {
    string actual = calculate_sha256(archive);

    if (actual != expected)
    {
      fs::remove(archive);
      throw ZCError(
          ZC_HASH_ERROR, "SECURITY ALERT: Hash mismatch!\nExpected: " + expected + "\nActual:   " + actual
      );
    }
    log_(LogLevel::SUCCESS, "Hash is correct");
  }
  catch (const ZCError &e)
  {
    throw ZCError(ZC_INTERNAL_ERROR, "Failed to compute hash: " + std::string(e.what()));
  }
}

void Controller::installPackageFromServer(
    const string &name, const string &version, const json &index, bool quiet, Visited &v
)
{
  string url = resolvePackageUrl(name, version, index);
  log_(LogLevel::INFO, "Downloading " + url + "...");
  const fs::path archive_path = tmp_dir_ / (name + ".tar.gz");
  const fs::path extract_path = tmp_dir_ / name;
  net_.download(url, archive_path);

  string version_key = version.empty() ? index["packages"][name]["latest"].get<std::string>() : version;
  verifyPackageHash(archive_path, index["packages"][name]["versions"][version_key]["sha256"]);

  extractAndInstall(archive_path, extract_path, quiet, v, false);
}

void Controller::installFromServer(Targets &targets, bool quiet)
{
  Visited v;
  installFromServer(targets, quiet, v);
}

void Controller::installFromServer(Targets &targets, bool quiet, Visited &visited)
{
  json index = net_.getIndex(log_);

  for (const auto &[name, version] : targets)
  {
    try
    {
      installPackageFromServer(name, version, index, quiet, visited);
    }
    catch (const ZCError &e)
    {
      log_(LogLevel::ERROR, e.what());
    }
  }
  clean();
}

void Controller::buildAndIndex(LocalController &pc, bool quiet, const std::string &origin, bool isUpdate)
{
  // Parameter = Controller for the project that is being installed
  if (!isUpdate && r_->pkgExists(pc.lc_->name_) && !force_)
    if (!ask(
            "The package '" + pc.lc_->name_ + "' seems to be already installed. Do you want to reinstall it?"
        ))
      return;
  pc.buildProject(quiet, true);

#ifdef DEBUG_MODE
  log_(LogLevel::DEBUG, "Project compiled");
#endif

  // Clean before update
  if (isUpdate && r_->pkgExists(pc.lc_->name_))
    removePackage(pc.lc_->name_);

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
  case Type::UNDEF: // Should never happen since already checked in pc.buildProject()
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
  if (isUpdate)
    log_(LogLevel::SUCCESS, "Package " + pc.lc_->name_ + " updated successfully.");
  else
    log_(LogLevel::SUCCESS, "Package " + pc.lc_->name_ + " installed successfully.");
}

void Controller::installLibrary(LocalController &pc)
{
  const fs::path dest_include = include_dir_ / pc.lc_->name_;
  const fs::path dest_lib = lib_dir_ / pc.lc_->name_;

  const fs::path src_include = pc.root_dir_ / pc.lc_->include_folder_;

  pc.log_(LogLevel::INFO, "Installing headers...");

  fs::create_directories(dest_include);
  fs::copy(src_include, dest_include, fs::copy_options::recursive | fs::copy_options::overwrite_existing);

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

  pc.log_(LogLevel::INFO, "Installing binary...");

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

  if (p.is_installed_locally)
  {
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

Table Controller::packagesTable() const
{
  vector<vector<string>> str_pkgs{{"Package name", "Target", "Origin", "Version", "Type"}};

  for (const auto &[name_, binary_, origin_, version_, is_bin_, _] : r_->getPackages())
    str_pkgs.push_back({name_, binary_, origin_, version_.string(), is_bin_ ? "Executable" : "Library"});

  return {static_cast<int>(str_pkgs.size()), 5, false, true, str_pkgs};
}

Table Controller::remotePackagesTable() const
{
  vector<vector<string>> str_pkgs{{"Package name"}};

  for (const auto &p : getRemotePackages()) str_pkgs.push_back({p});

  return {static_cast<int>(str_pkgs.size()), 1, false, true, str_pkgs};
}

void Controller::saveRegistry()
{
  r_->write();
}

vector<Package> Controller::getPackages() const
{
  return r_->getPackages();
}

Package Controller::getPackage(const std::string &pkg) const
{
  return r_->getPackage(pkg);
}

std::vector<std::string> Controller::getRemotePackages() const
{
  json index = net_.getIndex(log_);
  vector<std::string> v;
  if (index.contains("packages") && index["packages"].is_object())
    for (auto it = index["packages"].begin(); it != index["packages"].end(); ++it) v.push_back(it.key());

  clean();
  return v;
}
