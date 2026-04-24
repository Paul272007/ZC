#include <algorithm>

#include "files.hh"
#include "nlohmann/json.hpp"
#include "objects/Registries/Registry.hh"
#include "objects/ZCError.hh"

using namespace std;
namespace fs = std::filesystem;
using json = nlohmann::json;

void from_json(const json &j, Package &p)
{
  if (!j.is_array())
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "Package format is invalid (expected array).");
  if (j.size() < 5)
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "Not enough values given in package declaration.");
  if (!j.at(0).is_string() || !j.at(1).is_string() || !j.at(2).is_string() || !j.at(3).is_string() ||
      !j.at(4).is_boolean())
    throw ZCError(ZC_CONFIG_TYPE_ERROR, "Incorrect data types in package declaration.");

  j.at(0).get_to(p.name);
  p.version = j.at(1).get<std::string>();
  j.at(2).get_to(p.binary);
  j.at(3).get_to(p.origin);
  j.at(4).get_to(p.is_exec);
  if (j.size() == 6)
    j.at(5).get_to(p.is_installed_locally);
}

void to_json(json &j, const Package &p)
{
  if (!p.is_installed_locally)
    j = json::array({p.name, p.version.string(), p.binary, p.origin, p.is_exec, p.is_installed_locally});
  else
    j = json::array({p.name, p.version.string(), p.binary, p.origin, p.is_exec});
}

void Registry::load()
{
  // Global registry has to exist
  if (!fs::exists(file_))
    return; // no file = no installed libraries / dependencies

  json json_registry = parseJsonFile(file_);

  if (json_registry.contains("libraries") && json_registry["libraries"].is_array())
    pkgs_ = json_registry.at("libraries").get<vector<Package>>();
}

void Registry::write() const
{
  json root;
  root["libraries"] = pkgs_;

  writeJsonFile(root, file_);
}

void Registry::indexPackage(const Package &package)
{
  if (const auto it = ranges::find_if(pkgs_, [&](const Package &p) { return p.name == package.name; });
      it != pkgs_.end())
    *it = package;
  else
    pkgs_.push_back(package);
}

Package Registry::unindexPackage(const std::string &pkg_name)
{
  if (const auto it = std::ranges::find_if(pkgs_, [&](const Package &p) { return p.name == pkg_name; });
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
  const auto it = ranges::find_if(pkgs_, [&](const Package &p) { return p.name == pkg_name; });
  return it != pkgs_.end();
}

const Package &Registry::getPackage(const std::string &pkg_name) const
{
  const auto it = ranges::find_if(pkgs_, [&](const Package &p) { return p.name == pkg_name; });
  if (it == pkgs_.end())
    throw ZCError(ZC_PACKAGE_NOT_FOUND, "The package " + pkg_name + " was not found");

  return *it;
}

const std::vector<Package> &Registry::getPackages() const
{
  return pkgs_;
}
