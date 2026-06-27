#include "Install.h"

#include <filesystem>

#include "commands/Command.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "pkgs/Network.h"
#include "project/Project.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

Install::Install(
  bool force, const std::filesystem::path &p_root, const filesystem::path &path,
  std::vector<std::string> &targets, const bool is_std
)
  : Command(force),
    p_root_(get_project_root(p_root)),
    path_(path),
    targets_(parse_targets(targets)),
    std_(is_std)
{
}

void Install::operator()()
{
  if (!path_.empty())
    install_from_path();
  elif (targets_.empty())
    install_dependencies();
  else
    install_targets();
}

void Install::install_from_path()
{
  if (!targets_.empty())
    throw ZCException(
      ZCE_INCOMPATIBLE_FLAGS, "Cannot install from remote and from local project at the same time"
    );

  reg_.install_from_path(path_, force_);
}

void Install::install_dependencies()
{
  Project(p_root_).install_dependencies();
}

void Install::install_targets()
{
  if (std_)
  {
    if (!has_pkg_config())
      throw ZCException(ZCE_NOT_FOUND, "Command 'pkg-config' is required to install standard packages");
    for (CAA target : targets_)
      reg_.install_std(target.name);
  }
  else
  {
    json index = Network::get().get_index();
    for (auto &target : targets_)
      reg_.install_from_server(target, index, force_);
  }
}

} // namespace zc
