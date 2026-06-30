#include "PConf.h"

#include <string>

#include "../helpers.h"
#include "Conf.h"
#include "config/Dependency.h"
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
  if (dependencies.contains(d.name))
    throw ZCException(ZCE_ALREADY_INSTALLED, "Dependency " + d.name + " already added");
  dependencies.insert_or_assign(d.name, d);
  modified_ = true;
}

void PConf::change_dependency_version(const std::string &dep_name, const Version &new_version)
{
  if (!dependencies.contains(dep_name))
    throw ZCException(ZCE_PKG_NOT_FOUND, "Dependency " + dep_name + " was not found.");
  dependencies.at(dep_name).version = new_version;
  modified_                         = true;
}

void PConf::remove_dependency(const std::string &dep_name)
{
  if (dependencies.erase(dep_name) == 0) // Returns 0 if not found
    throw ZCException(ZCE_PKG_NOT_FOUND, "Dependency " + dep_name + " was not found.");
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
  get_key(root, "macros", macros, macros);

  if (version.is_empty())
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
    dependencies.clear();
    for (CAA[key, value] : root["dependencies"].items())
    {
      auto d = value.get<Dependency>();
      d.name = key;
      dependencies.insert_or_assign(key, d);
    }
  }
}

void PConf::write()
{
  json root;

  root["type"]         = type;
  root["macros"]       = macros;
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
  for (CAA[lang, conf] : languages)
    lang_json[language_to_str(lang)] = conf;
  root["languages"] = lang_json;

  json deps_json = json::object();
  for (CAA[dep_name, conf] : dependencies)
    deps_json[dep_name] = conf;
  root["dependencies"] = deps_json;

  write_json(root, file_);
}

} // namespace zc
