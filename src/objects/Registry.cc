#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <commands/Build.hh>
#include <helpers.hh>
#include <interface.hh>
#include <nlohmann/json.hpp>
#include <objects/ProjectSettings.hh>
#include <objects/Registry.hh>
#include <objects/ZCError.hh>

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;

Registry::Registry()
    : registry_path_(getZCRootDir() / REGISTRY), include_path_(getZCRootDir() / INCLUDE_DIR),
      lib_path_(getZCRootDir() / LIB_DIR), bin_path_(getZCRootDir() / BIN_DIR)
{
  load(true);
}

Registry::Registry(const std::filesystem::path &project_root)
    : registry_path_(project_root / REGISTRY), include_path_(project_root / EXTERNAL / INCLUDE_DIR),
      lib_path_(project_root / EXTERNAL / LIB_DIR), bin_path_(project_root / EXTERNAL / BIN_DIR)
{
  load(false);
}

void from_json(const json &j, Package &p)
{
  if (!j.is_array())
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "Package format is invalid (expected array).");
  if (j.size() < N_ATTR_PKG)
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "Not enough values given in package declaration.");
  if (!j.at(0).is_string() || !j.at(1).is_string() || !j.at(2).is_string() || !j.at(3).is_string() ||
      !j.at(4).is_boolean())
    throw ZCError(ZC_CONFIG_TYPE_ERROR, "Incorrect data types in package declaration.");
  j.at(0).get_to(p.name_);
  j.at(1).get_to(p.version_);
  j.at(2).get_to(p.binary_);
  j.at(3).get_to(p.origin_);
  j.at(4).get_to(p.is_bin_);
}

void to_json(json &j, const Package &p)
{
  j = json::array({p.name_, p.version_, p.binary_, p.origin_, p.is_bin_});
}

void Registry::load(bool is_global)
{
  // Global registry has to exist
  if (!is_global && !fs::exists(registry_path_))
    return; // no file = no installed libraries / dependencies

  json json_registry = parseJsonFile(registry_path_);

  if (json_registry.contains("libraries") && json_registry["libraries"].is_array())
    pkgs_ = json_registry.at("libraries").get<vector<Package>>();
}

void Registry::write() const
{
  json root;
  root["libraries"] = pkgs_;

  writeJsonFile(root, registry_path_);
}

void Registry::installExecutable(const Build &b, bool quiet)
{
  const fs::path exe_dest = bin_path_ / b.p_settings_.target_name_;
  const fs::path exe_source = b.p_settings_.root_ / BUILD_DIR / b.p_settings_.target_name_;

  if (!quiet)
    info("Installing global binary...");

  fs::create_directories(bin_path_);

  if (!fs::exists(exe_source))
    throw ZCError(ZC_NOT_FOUND, "The compiled binary was not found: " + exe_source.string());

  fs::copy_file(exe_source, exe_dest, fs::copy_options::overwrite_existing);

  fs::permissions(
      exe_dest, fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec, fs::perm_options::add
  );
}

void Registry::installLibrary(const Build &b, bool quiet)
{
  const fs::path dest_include = include_path_ / b.p_settings_.name_;
  const fs::path dest_lib = lib_path_ / b.p_settings_.name_;

  if (!quiet)
    info("Installing headers...");

  fs::create_directories(dest_include);
  fs::copy(
      b.p_settings_.include_folder_, dest_include,
      fs::copy_options::recursive | fs::copy_options::overwrite_existing
  );

  if (!quiet)
    info("Installing libraries...");

  fs::create_directories(dest_lib);

  for (const auto &entry : fs::recursive_directory_iterator(b.p_settings_.root_ / BUILD_DIR))
  {
    if (string filename = entry.path().filename().string();
        filename.find(b.p_settings_.target_name_) != string::npos &&
        (entry.path().extension() == ".a" || entry.path().extension() == ".so" ||
         entry.path().extension() == ".dylib" || entry.path().extension() == ".lib"))
    {
      fs::copy_file(entry.path(), dest_lib / filename, fs::copy_options::overwrite_existing);
    }
  }
}

void Registry::installPackage(
    const std::filesystem::path &project_root, const bool force, const bool quiet, const std::string &origin
)
{
  Build b(true, quiet, false, project_root, false);

  if (pkgExists(b.p_settings_.name_) && !force)
    if (!ask(
            "The package '" + b.p_settings_.name_ +
            "' seems to be already installed. Do you want to reinstall it?"
        ))
      return;

  b();

#ifdef DEBUG_MODE
  if (!quiet)
    debug("Project compiled");
#endif

  bool is_bin = false;
  switch (b.p_settings_.type_)
  {
  case LIB:
    installLibrary(b, quiet);
    is_bin = false;
    break;
  case BIN:
    installExecutable(b, quiet);
    is_bin = true;
    break;
  case UNDEF:
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "The project type must be 'lib' or 'bin'");
  }

  indexPackage(
      Package{
          b.p_settings_.name_, b.p_settings_.version_->string(), b.p_settings_.target_name_, origin, is_bin
      }
  );
  if (!quiet)
    success("Package " + b.p_settings_.name_ + " installed successfully.");
}

bool Registry::removePackage(const std::string &pkg_name)
{
  Package p;
  try
  {
    p = unindexPackage(pkg_name);
  }
  catch (const ZCError &)
  {
    return false;
  }

  if (p.is_bin_)
  {
    if (fs::exists(bin_path_ / p.binary_))
      fs::remove(bin_path_ / p.binary_);
  }
  else
  {
    if (fs::exists(include_path_ / pkg_name))
      fs::remove_all(include_path_ / pkg_name);

    if (fs::exists(lib_path_ / pkg_name))
      fs::remove_all(lib_path_ / pkg_name);
  }
  return true;
}

void Registry::indexPackage(const Package &package)
{
  if (const auto it = ranges::find_if(pkgs_, [&](const Package &p) { return p.name_ == package.name_; });
      it != pkgs_.end())
    *it = package;
  else
    pkgs_.push_back(package);
}

Package Registry::unindexPackage(const std::string &pkg_name)
{
  if (const auto it = std::ranges::find_if(pkgs_, [&](const Package &p) { return p.name_ == pkg_name; });
      it != pkgs_.end())
  {
    Package removed_pkg = *it;
    pkgs_.erase(it);
    return removed_pkg;
  }
  else
    throw ZCError(ZC_PACKAGE_NOT_FOUND, "The package was not found: " + pkg_name);
}

bool Registry::pkgExists(const std::string &pkg_name) const
{
  const auto it = ranges::find_if(pkgs_, [&](const Package &p) { return p.name_ == pkg_name; });
  return it != pkgs_.end();
}

Table Registry::packagesTable() const
{
  vector<vector<string>> str_pkgs{{"Package name", "Version", "Target", "Origin", "Type"}};

  for (const auto &[name_, version_, binary_, origin_, is_bin_] : pkgs_)
    str_pkgs.push_back({name_, version_, binary_, origin_, is_bin_ ? "Executable" : "Library"});

  return {static_cast<int>(str_pkgs.size()), N_ATTR_PKG, false, true, str_pkgs};
}

const Package &Registry::getPackage(const std::string &pkg_name) const
{
  const auto it = ranges::find_if(pkgs_, [&](const Package &p) { return p.name_ == pkg_name; });
  if (it == pkgs_.end())
    throw ZCError(ZC_PACKAGE_NOT_FOUND, "The package " + pkg_name + " was not found");

  return *it;
}

const std::vector<Package> &Registry::getPackages() const
{
  return pkgs_;
}

const std::filesystem::path &Registry::getIncludePath() const
{
  return include_path_;
}

const std::filesystem::path &Registry::getLibPath() const
{
  return lib_path_;
}
