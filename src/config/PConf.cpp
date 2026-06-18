/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include <sys/stat.h>

#include "../helpers.h"
#include "Conf.h"
#include "PConf.h"

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
  json root = if_.read_json(file_);
  get_key(root, "name", name); // TODO : verify the name of the package here
  get_key(root, "author", author);
  get_key(root, "target", target);
  get_key(root, "c_std", c_std);
  get_key(root, "cxx_std", cxx_std);
  get_key(root, "c_compiler", c_compiler);
  get_key(root, "cxx_compiler", cxx_compiler);
  get_key(root, "flags", flags);
  get_key(root, "languages", languages);

  get_key(root, "type", type); // TODO : maybe throw an error if type is UNDEF
  get_key(root, "version", version);

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

  root["c_std"] = c_std;
  root["cxx_std"] = cxx_std;
  root["c_compiler"] = c_compiler;
  root["cxx_compiler"] = cxx_compiler;
  root["type"] = type;
  root["version"] = version;
  root["flags"] = flags;
  root["languages"] = languages;

  if (!name.empty())
    root["name"] = name;
  if (!root.empty())
    root["author"] = author;
  if (!target.empty())
    root["target"] = target;

  json deps_json = json::object();
  for (const auto &dep : dependencies)
    deps_json[dep.name] = {{"static_link", dep.static_link}, {"version", dep.version}};

  root["dependencies"] = deps_json;

  if_.write_json(root, file_);
}

} // namespace zc
