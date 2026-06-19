#include "commands/Init.h"
#include "commands/Command.h"
#include "config/PConf.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "pkgs/PkgType.h"
#include "templates/TemplateEngine.h"
#include <filesystem>

ZC_DEV_CONFIG

namespace zc
{

Init::Init(
    const bool force, const std::filesystem::path &p_root, bool git, bool edit, const std::string &author,
    const std::string &target, const std::string &p_template, const std::string &name, bool is_bin,
    bool is_lib, bool is_header, bool is_compose
)
    : Command(force), p_root_(p_root.empty() ? fs::current_path() : p_root), git_(git), edit_(edit),
      author_(author), p_template_(p_template), name_(name)
{
  type_ = parse_mode<PkgType>(
      {{BIN, is_bin}, {LIB, is_lib}, {HEADER, is_header}, {COMPOSE, is_compose}}, UNDEF,
      "Project cannot have multiple types"
  );
}

int Init::operator()()
{
  if (!force_ && fs::exists(ZC_FILE))
    if (!if_.ask(
            "It seems like a ZC project already exists in this directory. Do you want to overwrite it ?"
        ))
      throw ZCException(ZCE_ABORTED, "Project creation aborted.");

  if (name_.empty())
    name_ = if_.input("Package name: ", fs::current_path().filename().string());

  if (target_.empty())
    target_ = if_.input("Package target: ", name_);

  if (author_.empty())
    author_ = if_.input("Package author: ", gconf_.username);

  if (p_template_.empty())
    p_template_ = if_.input("Project template to use: ");

  if (!p_template_.empty())
    TemplateEngine::get().init_with_p_template(p_root_, p_template_, force_);

  if (git_)
  {
    if_.info("Initializing git repo...");
    if (system("git init") != 0)
      throw ZCException(ZCE_GIT_ERROR, "Git init failed");
  }

  PConf pconf(p_root_ / ZC_FILE);

  pconf.name = name_;
  pconf.target = target_;
  pconf.author = author_;
  pconf.type = type_;

  // TODO : replace with function to send command as array/vector and escape arguments
  if (gconf_.open_after_init || edit_)
    return system(string(escape_shell_arg(gconf_.editor) + " " + escape_shell_arg(p_root_.string())).c_str());

  return 0;
}

} // namespace zc
