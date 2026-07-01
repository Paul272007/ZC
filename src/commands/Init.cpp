#include "commands/Init.h"

#include <filesystem>
#include <utility>
#include <vector>

#include "commands/Command.h"
#include "config/Language.h"
#include "config/PConf.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "pkgs/PkgType.h"
#include "templates/TemplateEngine.h"
#include "ui/ui_utils.h"

ZC_DEV_CONFIG

namespace zc
{

Init::Init(
  const bool force, const std::filesystem::path &p_root, const bool git, const bool edit,
  std::string author, std::string target, std::string p_template, std::string name, bool is_bin,
  bool is_lib, bool is_header, bool is_compose, const vector<string> &languages
)
  : Command(force),
    p_root_(p_root.empty() ? fs::current_path() : p_root),
    git_(git),
    edit_(edit),
    name_(std::move(name)),
    target_(std::move(target)),
    author_(std::move(author)),
    p_template_(std::move(p_template))
{
  type_ = parse_mode<PkgType>(
    {
      { PkgType::BIN, is_bin },
      { PkgType::LIB, is_lib },
      { PkgType::HEADER, is_header },
      { PkgType::COMPOSE, is_compose },
    },
    PkgType::UNDEF, "Project cannot have multiple types"
  );

  for (const auto &l_str : languages)
    if (const auto l = language_from_str(l_str); l != UNKNOWN_LANGUAGE)
      languages_.push_back(l);
    else
      throw ZCException(ZCE_UNSUPPORTED_LANGUAGE, "Unknown language given: " + l_str);
}

void Init::operator()()
{
  if (fs::exists(p_root_ / ZC_FILE))
  {
    if (!force_ &&
        !if_.ask("A ZC project seems to already exist in this directory. Do you want to overwrite it ?"))
      throw ZCException(ZCE_ABORTED, "Project creation aborted.");
    fs::remove(p_root_ / ZC_FILE);
  }

  if (name_.empty())
    name_ = if_.input("Package name", fs::current_path().filename().string());

  if (target_.empty())
    target_ = if_.input("Package target", name_);

  if (author_.empty())
    author_ = if_.input("Package author", gc_.username);

  if (p_template_.empty())
    p_template_ = choose_p_template();

  if (type_ == PkgType::UNDEF)
    type_ = choose_pkg_type();

  te_.init_with_p_template(p_root_, p_template_, force_);

  if (languages_.empty())
  {
    vector<string> options;
    for (const auto &key : gc_.languages | views::keys)
      options.push_back(language_to_str(key));
    for (const vector<string> results = if_.checkboxes("Package language(s):", options);
         const auto          &result : results)
      languages_.push_back(language_from_str(result));
  }

  if (git_ && system("git init") != 0)
    throw ZCException(ZCE_GIT_ERROR, "Git init failed");

  PConf pconf(p_root_ / ZC_FILE);

  pconf.name   = name_;
  pconf.target = target_;
  pconf.author = author_;
  pconf.type   = type_;
  if (type_ == PkgType::BIN)
    pconf.include_dirs = { SRC_DIR };
  elif (type_ == PkgType::LIB || type_ == PkgType::HEADER)
    pconf.include_dirs = { INCLUDE_DIR };

  pconf.languages.clear();
  for (const auto l : languages_)
    if (!gc_.languages.contains(l))
      throw ZCException(
        ZCE_UNSUPPORTED_LANGUAGE, "No configuration available for language: " + language_to_str(l)
      );
    else
      pconf.languages.insert_or_assign(l, gc_.languages.at(l));

  if (gc_.open_after_init || edit_)
    open_project_in_editor(p_root_);
}

} // namespace zc
