#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

#include <commands/Build.hh>
#include <interface.hh>
#include <nlohmann/json.hpp>
#include <objects/ProjectSettings.hh>
#include <objects/Registry.hh>
#include <objects/ZCError.hh>

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;

Registry::Registry()
{
  load();
}

Registry &Registry::getInstance()
{
  static Registry instance;
  return instance;
}

void from_json(const json &j, StdPackage &p)
{
  if (!j.is_array())
    return;
  if (j.size() < 4)
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "Not enough values given.");
  j.at(0).get_to(p.name_);
  j.at(1).get_to(p.headers_);
  j.at(2).get_to(p.binaries_);
  j.at(3).get_to(p.flags_);
}

void from_json(const json &j, Package &p)
{
  if (!j.is_array())
    return;
  if (j.size() < 4)
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "Not enough values given.");
  j.at(0).get_to(p.name_);
  j.at(1).get_to(p.version_);
  j.at(2).get_to(p.author_);
  j.at(3).get_to(p.flags_);
}

void to_json(json &j, const StdPackage &p)
{
  j = json::array({p.name_, p.headers_, p.binaries_, p.flags_});
}

void to_json(json &j, const Package &p)
{
  j = json::array({p.name_, p.version_, p.author_, p.flags_});
}

void Registry::load()
{
  json json_registry;
  if (!fs::exists(registry_path_))
  {
    throw ZCError(ZC_CONFIG_NOT_FOUND, "The configuration file was not found: " + registry_path_.string());
  }
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
  if (json_registry.contains("libraries"))
    packages_ = json_registry.at("libraries").get<vector<Package>>();
  if (json_registry.contains("std_libraries"))
    std_packages_ = json_registry.at("std_libraries").get<vector<StdPackage>>();
}

void Registry::installPackage(std::filesystem::path &project_root, const bool force, const bool quiet)
{
  ProjectSettings p(project_root);

  if (p.getType() != LIB)
    throw ZCError();

  Build b(true, quiet, p);
  b.execute();

#ifdef DEBUG_MODE
  if (!quiet)
    debug("Projet compiled");
#endif

  fs::path global_zc = getZCRootDir();
  fs::path dest_include = global_zc / "include" / p.getName();
  fs::path dest_lib = global_zc / "lib" / p.getName();

  if ((fs::exists(dest_include) || fs::exists(dest_lib) || pkgExists(p.getName())) && !force)
    if (!ask("The library seems to be already installed on this machine. Do you want to overwrite it ?"))
      return;

  if (!quiet)
    info("Installing headers...");

  fs::create_directories(dest_include);
  fs::copy(
      p.getIncludeFolder(), dest_include, fs::copy_options::recursive | fs::copy_options::overwrite_existing
  );

  if (!quiet)
    info("Installing libraries...");

  fs::create_directories(dest_lib);
  vector<string> lib_names = {p.getStaticLibName(), p.getSharedLibName()};

  for (const string &name : lib_names)
  {
    if (name.empty())
      continue;

    for (const auto &entry : fs::recursive_directory_iterator(project_root / "build"))
    {
      string filename = entry.path().filename().string();

      if (filename.find(name) != string::npos &&
          (entry.path().extension() == ".a" || entry.path().extension() == ".so" ||
           entry.path().extension() == ".dylib" || entry.path().extension() == ".lib"))
      {
        fs::copy_file(entry.path(), dest_lib / filename, fs::copy_options::overwrite_existing);
      }
    }
  }
  string flags = "-l" + (p.getSharedLibName().empty() ? p.getStaticLibName() : p.getSharedLibName());
  indexPackage(Package{p.getName(), p.getAuthor(), p.getVersion().to_string(), flags});
  success("Package " + p.getName() + " installed successfully.");
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

bool Registry::pkgExists(const std::string &pkg_name) const
{
  const auto it = ranges::find_if(packages_, [&](const Package &p) { return p.name_ == pkg_name; });
  return it != packages_.end();
}

void Registry::indexPackage(const Package &package)
{
  packages_.push_back(package);
  write();
}

void Registry::unindexPackage(const std::string &pkg_name)
{
  auto it = ranges::find_if(packages_, [&](const Package &p) { return p.name_ == pkg_name; });

  if (it != packages_.end())
    packages_.erase(it);
  else
    throw ZCError(ZC_PACKAGE_NOT_FOUND, "The package was not found: " + pkg_name);

  write();
}

void Registry::write() const
{
  json root;
  root["libraries"] = packages_;
  root["std_libraries"] = std_packages_;
  ofstream output(registry_path_);
  if (!output.is_open())
    throw ZCError(ZC_CONFIG_WRITING_ERROR, "The registry couldn't be written: " + registry_path_.string());
  output << root.dump(4);
  output.close();
}

Table Registry::packagesTable() const
{
  vector<vector<string>> str_pkgs{{"Package name", "Author", "Version", "Compiling flags"}};

  for (const auto &[name_, author_, version_, flags_] : packages_)
    str_pkgs.push_back({name_, author_, version_, flags_});

  return Table(str_pkgs.size(), N_ATTR_PACKAGE, false, true, str_pkgs);
}

Table Registry::stdPackagesTable() const
{
  vector<vector<string>> str_pkgs{{"Package name", "Compiling flags", "Headers", "Binaries"}};

  for (const auto &[name_, headers_, binaries_, flags_] : std_packages_)
    str_pkgs.push_back({name_, flags_, join(headers_, ", "), join(binaries_, ", ")});

  return Table(str_pkgs.size(), N_ATTR_STD_PACKAGE, false, true, str_pkgs);
}

fs::path Registry::getIncludeDir() const
{
  return include_path_;
}

fs::path Registry::getLibDir() const
{
  return lib_path_;
}

std::vector<Package> Registry::getPackages() const
{
  return packages_;
}

std::vector<StdPackage> Registry::getStdPackages() const
{
  return std_packages_;
}
