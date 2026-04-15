#include <algorithm>
#include <fstream>
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

Registry::Registry(const bool is_global)
{
  if (is_global)
  {
    const fs::path &root(getZCRootDir());
    registry_path_ = root / REGISTRY;
    include_path_ = root / INCLUDE_DIR;
    lib_path_ = root / LIB_DIR;
  }
  else
  {
    const fs::path &root(getProjectRoot());
    registry_path_ = root / REGISTRY;
    include_path_ = root / EXTERNAL / INCLUDE_DIR;
    lib_path_ = root / EXTERNAL / LIB_DIR;
  }
  load();
}

void from_json(const json &j, StdPackage &p)
{
  if (!j.is_array())
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "Package format is invalid (expected array).");
  if (j.size() < N_ATTR_STD_PKG)
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "Not enough values given.");
  j.at(0).get_to(p.name_);
  j.at(1).get_to(p.headers_);
  j.at(2).get_to(p.flags_);
}

void from_json(const json &j, Package &p)
{
  if (!j.is_array())
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "Package format is invalid (expected array).");
  if (j.size() < N_ATTR_PKG)
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "Not enough values given.");
  j.at(0).get_to(p.name_);
  j.at(1).get_to(p.version_);
  j.at(2).get_to(p.shared_);
  j.at(3).get_to(p.static_);
}

void to_json(json &j, const StdPackage &p)
{
  j = json::array({p.name_, p.headers_, p.flags_});
}

void to_json(json &j, const Package &p)
{
  j = json::array({p.name_, p.version_, p.shared_, p.static_});
}

void Registry::load()
{
  json json_registry;
  if (!fs::exists(registry_path_))
    return; // = no installed libraries / dependencies
  // throw ZCError(ZC_CONFIG_NOT_FOUND, "The configuration file was not found: " + registry_path_.string());

  ifstream input(registry_path_);
  if (!input.is_open())
    throw ZCError(
        ZC_CONFIG_READING_ERROR, "The configuration file couldn't be read: " + registry_path_.string()
    );
  try
  {
    input >> json_registry;
  }
  catch (const json::parse_error &e)
  {
    throw ZCError(
        ZC_CONFIG_PARSING_ERROR,
        "The configuration file couldn't be parsed: " + registry_path_.string() + ": " + e.what()
    );
  }
  if (json_registry.contains("libraries") && json_registry["libraries"].is_array())
    pkgs_ = json_registry.at("libraries").get<vector<Package>>();

  if (json_registry.contains("std_libraries") && json_registry["libraries"].is_array())
    std_pkgs_ = json_registry.at("std_libraries").get<vector<StdPackage>>();
}

void Registry::write() const
{
  json root;
  root["libraries"] = pkgs_;
  root["std_libraries"] = std_pkgs_;
  ofstream output(registry_path_);
  if (!output.is_open())
    throw ZCError(ZC_CONFIG_WRITING_ERROR, "The registry couldn't be written: " + registry_path_.string());
  output << root.dump(2);
  output.close();
}

void Registry::installPackage(const std::filesystem::path &project_root, const bool force, const bool quiet)
{
  Build b(true, quiet, project_root);

  if (b.p_settings_.type_ != LIB)
    throw ZCError();

  const fs::path dest_include = include_path_ / b.p_settings_.name_;
  const fs::path dest_lib = lib_path_ / b.p_settings_.name_;

  if ((fs::exists(dest_include) || fs::exists(dest_lib) || pkgExists(b.p_settings_.name_)) && !force)
    if (!ask("The library seems to be already installed. Do you want to reinstall it ?"))
      return;

  b();

#ifdef DEBUG_MODE
  if (!quiet)
    debug("Project compiled");
#endif

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

  for (const vector lib_names = {b.p_settings_.static_lib_name_, b.p_settings_.shared_lib_name_};
       const string &name : lib_names)
  {
    if (name.empty())
      continue;

    for (const auto &entry : fs::recursive_directory_iterator(project_root / "build"))
    {
      if (string filename = entry.path().filename().string();
          filename.find(name) != string::npos &&
          (entry.path().extension() == ".a" || entry.path().extension() == ".so" ||
           entry.path().extension() == ".dylib" || entry.path().extension() == ".lib"))
      {
        fs::copy_file(entry.path(), dest_lib / filename, fs::copy_options::overwrite_existing);
      }
    }
  }
  indexPackage(
      Package{
          b.p_settings_.name_, b.p_settings_.version_->string(), b.p_settings_.shared_lib_name_,
          b.p_settings_.static_lib_name_
      }
  );
  success("Package " + b.p_settings_.name_ + " installed successfully.");
}

bool Registry::removePackage(const std::string &pkg_name)
{
  unindexPackage(pkg_name);

  if (!fs::exists(include_path_ / pkg_name) || !fs::exists(lib_path_ / pkg_name))
    return false;

  fs::remove_all(include_path_ / pkg_name);
  fs::remove_all(lib_path_ / pkg_name);
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

void Registry::unindexPackage(const std::string &pkg_name)
{
  if (const auto it = ranges::find_if(pkgs_, [&](const Package &p) { return p.name_ == pkg_name; });
      it != pkgs_.end())
    pkgs_.erase(it);
  else
    throw ZCError(ZC_PACKAGE_NOT_FOUND, "The package was not found: " + pkg_name);
}

bool Registry::pkgExists(const std::string &pkg_name) const
{
  const auto it = ranges::find_if(pkgs_, [&](const Package &p) { return p.name_ == pkg_name; });
  if (it != pkgs_.end())
    return true;

  const auto it2 = ranges::find_if(std_pkgs_, [&](const StdPackage &sp) { return sp.name_ == pkg_name; });
  return it2 == std_pkgs_.end();
}

Table Registry::packagesTable() const
{
  vector<vector<string>> str_pkgs{{"Package name", "Version", "Shared library name", "Static library name"}};

  for (const auto &[name_, version_, shared_, static_] : pkgs_)
    str_pkgs.push_back({name_, version_, shared_, static_});

  return {static_cast<int>(str_pkgs.size()), N_ATTR_PKG, false, true, str_pkgs};
}

Table Registry::stdPackagesTable() const
{
  vector<vector<string>> str_pkgs{{"Package name", "Compiling flags", "Headers"}};

  for (const auto &[name_, headers_, flags_] : std_pkgs_)
    str_pkgs.push_back({name_, flags_, join(headers_, ", ")});

  return {static_cast<int>(str_pkgs.size()), N_ATTR_STD_PKG, false, true, str_pkgs};
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

const std::vector<StdPackage> &Registry::getStdPackages() const
{
  return std_pkgs_;
}

const std::filesystem::path &Registry::getIncludePath() const
{
  return include_path_;
}

const std::filesystem::path &Registry::getLibPath() const
{
  return lib_path_;
}
