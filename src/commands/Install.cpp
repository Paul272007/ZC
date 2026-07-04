#include "Install.h"

#include <filesystem>

#include "commands/InstallCommand.h"
#include "helpers.h"
#include "pkgs/RemoteTarget.h"
#include "project/Project.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

Install::Install(
  CommandContext &c_ctx, const BuildContext &b_ctx, InstallContext &i_ctx, bool is_std, bool save_path
)
  : BuildCommand(b_ctx), InstallCommand(c_ctx, i_ctx), std_(is_std), save_path_(save_path)
{
}

void Install::operator()()
{
  if (has_project())
    install_dependencies();
  elif (!path_.empty())
    install_from_path();
  else
    install_targets();
}

void Install::install_from_path()
{
  if (!targets_.empty())
    throw ZCException(
      ZCE_INCOMPATIBLE_FLAGS, "Cannot install from remote and from local project at the same time"
    );

  Project project = reg_.install_from_path(path_, force_, save_path_, jobs_);
  if (sync_)
    p().add_dependency(LocalTarget::get_target({ project.pconf.name, project.pconf.version }));
}

void Install::install_dependencies()
{
  p().install_dependencies(force_);
}

void Install::install_targets()
{
  if (std_)
  {
    if (!has_pkg_config())
      throw ZCException(ZCE_NOT_FOUND, "Command 'pkg-config' is required to install standard packages");
    for (CAA[name, version] : targets_)
      reg_.install_std(name);
  }
  else
  {
    for (CAA target : targets_)
      reg_.install_from_server(RemoteTarget::get_target(target), force_, jobs_);
  }
  if (sync_)
    for (CAA target : targets_)
      p().add_dependency(LocalTarget::get_target(target));
}

} // namespace zc
