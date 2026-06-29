#include "Install.h"

#include <filesystem>
#include <utility>

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
  const bool force, const fs::path &p_root, fs::path path, const vector<string> &targets, const bool is_std
)
  : Command(force),
    p_root_(targets.empty() && path.empty() ? get_project_root(p_root) : fs::current_path()),
    path_(std::move(path)),
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

void Install::install_from_path() const
{
  if (!targets_.empty())
    throw ZCException(
      ZCE_INCOMPATIBLE_FLAGS, "Cannot install from remote and from local project at the same time"
    );

  reg_.install_from_path(path_, force_);
}

void Install::install_dependencies() const
{
  Project(p_root_).install_dependencies();
}

void Install::install_targets()
{
  if (std_)
  {
    if (!has_pkg_config())
      throw ZCException(ZCE_NOT_FOUND, "Command 'pkg-config' is required to install standard packages");
    for (CAA[name, _] : targets_)
      reg_.install_std(name);
  }
  else
  {
    const json index = Network::get().get_index();
    for (auto &target : targets_)
      reg_.install_from_server(target, index, force_);
  }
}

} // namespace zc
