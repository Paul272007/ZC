/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include <string>
#include <sys/stat.h>

#include "../helpers.h"
#include "Conf.h"
#include "PConf.h"
#include "config/LanguageConf.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

/**
 * PConf implementation
 *
 * Project configuration
 */

PConf::~PConf()
{
  if (modified_)
    PConf::write();
}

void PConf::add_dependency(const Dependency &d)
{
  dependencies.push_back(d);
  modified_ = true;
}

void PConf::remove_dependency(const std::string &dep_name)
{
  const auto it =
      ranges::find_if(dependencies, [dep_name](const Dependency &d) { return d.name == dep_name; });

  if (it == dependencies.end())
    throw ZCException(ZCE_PKG_NOT_FOUND, "Dependency " + dep_name + " was not found.");

  dependencies.erase(it);
  modified_ = true;
}

/**
 * @param file
 */
PConf::PConf(const std::filesystem::path &file) : Conf(file)
{
  if (fs::exists(file_))
    PConf::load();
  else // this means no configuration exists so it needs to be written
    modified_ = true;
}

void PConf::load()
{
  const json root = if_.read_json(file_);
  get_key(root, "name", name);
  get_key(root, "author", author, author);
  get_key(root, "target", target, name);
  get_key(root, "src_dirs", src_dirs, src_dirs);
  get_key(root, "include_dirs", include_dirs, include_dirs);
  get_key(root, "type", type);
  get_key(root, "version", version);

  if (type == UNDEF)
    throw ZCException(ZCE_CONTENT_ERROR, "Package type cannot be UNDEF");

  check_name(name);
  if (target != name)
    check_name(target);

  // Get languages configuration
  if (root.contains("languages") && root["languages"].is_object())
  {
    languages.clear();
    for (const auto &[key, value] : root["languages"].items())
    {
      LanguageConf l = value.get<LanguageConf>();
      l.name = language_from_str(key);
      languages.push_back(l);
    }
  }

  // Get dependencies
  if (root.contains("dependencies") && root["dependencies"].is_object())
  {
    for (const auto &[key, value] : root["dependencies"].items())
    {
      Dependency d;
      d.name = key;
      get_key(value, "static_link", d.static_link);
      get_key(value, "version", d.version);
      dependencies.push_back(d);
    }
  }
}

void PConf::write()
{
  json root;

  root["type"] = type;
  root["version"] = version;
  root["src_dirs"] = src_dirs;
  root["include_dirs"] = include_dirs;

  if (!name.empty())
    root["name"] = name;
  if (!root.empty())
    root["author"] = author;
  if (!target.empty())
    root["target"] = target;

  json lang_json = json::object();
  for (const auto &l : languages) lang_json[language_to_str(l.name)] = l;
  root["languages"] = lang_json;

  json deps_json = json::object();
  for (const auto &dep : dependencies)
    deps_json[dep.name] = {{"static_link", dep.static_link}, {"version", dep.version}};
  root["dependencies"] = deps_json;

  if_.write_json(root, file_);
}

} // namespace zc
