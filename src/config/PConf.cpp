#include "PConf.h"

#include <algorithm>
#include <string>
#include <sys/stat.h>

#include "../helpers.h"
#include "Conf.h"
#include "config/GConf.h"
#include "config/LanguageConf.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

PConf::~PConf()
{
  if (modified_)
    PConf::write();
}

void PConf::add_dependency(const Dependency &d)
{
  // Already checked in Project::add_dependency() if pkg is installed
  if (ranges::find_if(dependencies, [&d](const Dependency &dep) { return d.name == dep.name; }) !=
      dependencies.end())
    throw ZCException(ZCE_ALREADY_INSTALLED, "Dependency " + d.name + " already added");

  dependencies.push_back(d);
  modified_ = true;
}

void PConf::change_dependency_version(const std::string &name, const Version &new_version)
{
  // Already checked in Project::change_dependency_version() if pkg is installed
  const auto it = ranges::find_if(dependencies, [&name](const Dependency &d) { return d.name == name; });

  if (it == dependencies.end())
    throw ZCException(ZCE_PKG_NOT_FOUND, "Dependency " + name + " was not found.");

  it->version = new_version;
  modified_   = true;
}

void PConf::remove_dependency(const std::string &dep_name)
{
  const auto it =
    ranges::find_if(dependencies, [&dep_name](const Dependency &d) { return d.name == dep_name; });

  if (it == dependencies.end())
    throw ZCException(ZCE_PKG_NOT_FOUND, "Dependency " + dep_name + " was not found.");

  dependencies.erase(it);
  modified_ = true;
}

PConf::PConf(const std::filesystem::path &file) : Conf(file), languages(GConf::get().languages)
{
  if (fs::exists(file_))
    PConf::load();
  else // this means no configuration exists so it needs to be written
    modified_ = true;
}

void PConf::load()
{
  const json root = read_json(file_);
  get_key(root, "name", name);
  get_key(root, "author", author, author);
  get_key(root, "target", target, name);
  get_key(root, "src_dirs", src_dirs, src_dirs);
  get_key(root, "include_dirs", include_dirs, include_dirs);
  get_key(root, "type", type);
  get_key(root, "version", version);

  if (version.empty())
    throw ZCException(ZCE_CONTENT_ERROR, "Version cannot be empty");

  if (type == PkgType::UNDEF)
    throw ZCException(ZCE_CONTENT_ERROR, "Package type cannot be undefined");

  check_name(name);
  if (target != name)
    check_name(target);

  // Get languages configuration
  if (root.contains("languages") && root["languages"].is_object())
  {
    languages.clear();
    for (const auto &[key, value] : root["languages"].items())
      languages.insert_or_assign(language_from_str(key), value.get<LanguageConf>());
  }

  // Get dependencies
  if (root.contains("dependencies") && root["dependencies"].is_object())
  {
    for (const auto &[key, value] : root["dependencies"].items())
    {
      Dependency d;
      d.name = key;
      get_key(value, "origin", d.origin, d.origin);
      get_key(value, "static_link", d.static_link, d.static_link);
      get_key(value, "version", d.version);
      dependencies.push_back(d);
    }
  }
}

void PConf::write()
{
  json root;

  root["type"]         = type;
  root["version"]      = version;
  root["src_dirs"]     = src_dirs;
  root["include_dirs"] = include_dirs;

  if (!name.empty())
    root["name"] = name;
  if (!root.empty())
    root["author"] = author;
  if (!target.empty())
    root["target"] = target;

  json lang_json = json::object();
  for (const auto &l : languages)
    lang_json[language_to_str(l.first)] = l;
  root["languages"] = lang_json;

  json deps_json = json::object();
  for (const auto &dep : dependencies)
    deps_json[dep.name] = {
      { "origin", dep.origin },
      { "static_link", dep.static_link },
      { "version", dep.version },
    };
  root["dependencies"] = deps_json;

  write_json(root, file_);
}

} // namespace zc
