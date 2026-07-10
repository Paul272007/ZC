#include "CConf.h"

#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "pkgs/PkgType.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

CConf::~CConf()
{
  if (modified_)
    CConf::write();
}

CConf::CConf(const std::filesystem::path &file) : Conf(file)
{
  if (fs::exists(file_))
    CConf::load();
  else // this means no configuration exists so it needs to be written
    modified_ = true;
}

void CConf::load()
{
  const json root = read_json(file_);

  get_key(root, "type", type);

  if (type == PkgType::UNDEF || type == PkgType::COMPOSE)
    throw ZCException(ZCE_CONTENT_ERROR, "Component type cannot be undefined or compose");

  get_key(root, "macros", macros, macros);
  get_key(root, "requires", required, required);
  get_key(root, "include_dirs", include_dirs, { type == PkgType::BIN ? SRC_DIR : INCLUDE_DIR });

  if (type != PkgType::HEADER)
  {
    get_key(root, "target", target);
    get_key(root, "src_dirs", src_dirs, { SRC_DIR });
    check_name(target);
  }

  if (root.contains("dependencies") && root["dependencies"].is_object())
  {
    dependencies.clear(); // just a security
    for (CAA[key, value] : root["dependencies"].items())
    {
      auto d = value.get<Dependency>();
      d.name = key;
      dependencies.insert_or_assign(key, d);
    }
  }
}

void CConf::write()
{
  json root;
  root["type"]         = type;
  root["macros"]       = macros;
  root["include_dirs"] = include_dirs;
  root["requires"]     = required;

  if (type != PkgType::HEADER)
  {
    root["src_dirs"] = src_dirs;
    if (!target.empty())
      root["target"] = target;
  }

  json deps_json = json::object();
  for (CAA[dep_name, conf] : dependencies)
    deps_json[dep_name] = conf;
  root["dependencies"] = deps_json;
}

void CConf::add_dependency(const Dependency &d)
{
  if (dependencies.contains(d.name))
    throw ZCException(ZCE_ALREADY_INSTALLED, "Dependency '" + d.name + "' already added");
  dependencies.insert_or_assign(d.name, d);
  modified_ = true;
}

void CConf::change_dependency_version(const std::string &dep_name, const Version &new_version)
{
  if (!dependencies.contains(dep_name))
    throw ZCException(ZCE_PKG_NOT_FOUND, "Dependency " + dep_name + " was not found.");
  dependencies.at(dep_name).version = new_version;
  modified_                         = true;
}

void CConf::remove_dependency(const std::string &dep_name)
{
  if (dependencies.erase(dep_name) == 0) // Returns 0 if not found
    throw ZCException(ZCE_PKG_NOT_FOUND, "Dependency " + dep_name + " was not found.");
  modified_ = true;
}

} // namespace zc
