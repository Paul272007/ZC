#include "PConf.h"

#include <ranges>
#include <string>

#include "config/Conf.h"
#include "config/Dependency.h"
#include "config/GConf.h"
#include "config/Language.h"
#include "config/LanguageConf.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "ui/Interface.h"
#include "ui/ui_utils.h"

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
    throw ZCException(ZCE_ALREADY_INSTALLED, "Dependency '" + d.name + "' already added");
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

bool PConf::has_language(Language l) const
{
  return languages.contains(l);
}

void PConf::show_language(Language l) const
{
  if (!has_language(l))
    throw ZCException(ZCE_NOT_FOUND, "Language " + language_to_str(l) + " not found.");

  const LanguageConf &lc = languages.at(l);
  ui().new_line();
  ui().print(PURPLE + language_to_str(l) + RESET ":");
  ui().info("Compiler: " + lc.compiler);
  ui().info("Standard: " + lc.std);
  ui().info("Flags:");
  for (CAA f : lc.flags)
    ui().print("   " + f);
}

void PConf::show_languages() const
{
  for (CAA l : languages | views::keys)
    show_language(l);
}

void PConf::add_language(Language l)
{
  // TODO: maybe prompt for config instead of taking from gconf ?
  if (!gc().languages.contains(l))
    throw ZCException(
      ZCE_UNSUPPORTED_LANGUAGE, "No configuration available for language: " + language_to_str(l)
    );
  languages.insert_or_assign(l, gc().languages.at(l));
  modified_ = true;
}

void PConf::remove_language(Language l)
{
  if (!languages.contains(l))
    throw ZCException(ZCE_NOT_FOUND, "Language " + language_to_str(l) + " not found.");
  languages.erase(l);
  modified_ = true;
}

void PConf::edit_language(Language l)
{
  if (!has_language(l))
    throw ZCException(ZCE_NOT_FOUND, "Language " + language_to_str(l) + " not found.");
  ui().info("Editing configuration for language " + language_to_str(l));

  LanguageConf &lc = languages.at(l);
  lc.compiler      = ui().input("Compiler to use", lc.compiler);
  lc.std           = ui().input("Standard to use", lc.std);
  lc.flags         = ui().input_list("Flags to add to the compiler", lc.flags);
  modified_        = true;
}

void PConf::edit_languages()
{
  vector<string> options;
  vector<bool>   selected;

  for (const auto &key : gc().languages | views::keys)
  {
    options.push_back(language_to_str(key));
    selected.push_back(has_language(key));
  }

  ui().checkboxes("Package languages:", options, selected);

  languages.clear();
  for (size_t i = 0; i < options.size(); i++)
    if (selected[i])
      add_language(language_from_str(options[i]));

  modified_ = true;
}

Table PConf::dependencies_table() const
{
  vector<vector<string>> str_deps = { { "Name", "Package origin", "Link type", "Version" } };
  for (const auto &d : dependencies | views::values)
    str_deps.push_back({ d.name, d.origin, d.static_link ? "Static" : "Dynamic", d.version.string() });

  return { false, true, str_deps };
}

} // namespace zc
