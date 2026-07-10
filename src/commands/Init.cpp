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
#include "ui/ShellCommand.h"
#include "ui/ui_utils.h"

ZC_DEV_CONFIG

namespace zc
{

Init::Init(
  const CommandContext &ctx, std::filesystem::path p_root, bool git, bool edit, std::string author,
  std::string target, std::string p_template, std::string name, bool is_bin, bool is_lib, bool is_header,
  bool is_compose, const std::vector<std::string> &languages
)
  : Command(ctx, false),
    p_root_(std::move(p_root)),
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

  PConf pconf(p_root_ / ZC_FILE);

  pconf.type = type_ == PkgType::UNDEF ? choose_pkg_type() : type_;
  pconf.name = name_.empty() ? if_.input("Package name", fs::current_path().filename().string()) : name_;
  check_name(pconf.name);
  pconf.author = author_.empty() ? if_.input("Package author", gc_.username) : author_;

  if (pconf.type != PkgType::COMPOSE && pconf.type != PkgType::HEADER)
  {
    pconf.target = target_.empty() ? if_.input("Package target", pconf.name) : target_;
    if (pconf.target != pconf.name)
      check_name(pconf.target);
    if (languages_.empty())
      pconf.edit_languages();
    else
    {
      pconf.languages.clear();
      for (const auto l : languages_)
        pconf.add_language(l);
    }
  }

  if (p_template_.empty())
    p_template_ = choose_p_template();
  te().init_with_p_template(p_root_, p_template_, force_);

  if (pconf.type == PkgType::BIN)
    pconf.include_dirs = { SRC_DIR };
  elif (pconf.type == PkgType::LIB || pconf.type == PkgType::HEADER)
    pconf.include_dirs = { INCLUDE_DIR };

  if (git_ && ShellCommand::exec({ "git", "init" }) != 0)
    throw ZCException(ZCE_GIT_ERROR, "Git init failed");

  if (gc_.open_after_init || edit_)
    open_project_in_editor(p_root_);
}

} // namespace zc
