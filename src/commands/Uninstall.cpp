#include "Uninstall.h"

#include "commands/InstallCommand.h"
#include "Context.h"
#include "helpers.h"
#include "pkgs/LocalTarget.h"

ZC_DEV_CONFIG

namespace zc
{

Uninstall::Uninstall(CommandContext &c_ctx, InstallContext &i_ctx) : InstallCommand(c_ctx, i_ctx) {}

void Uninstall::operator()()
{
  if (has_project())
    uninstall_dependencies();
  elif (!path_.empty())
    uninstall_from_path();
  else
    uninstall_targets();
}

void Uninstall::uninstall_targets()
{
  for (const auto &target : targets_)
    if (target.second.is_empty())
      reg_.uninstall(target.first, force_);
    else
      reg_.uninstall(LocalTarget::get_target(target), force_);
}

void Uninstall::uninstall_from_path()
{
  reg_.uninstall_from_path(path_, force_);
}

void Uninstall::uninstall_dependencies()
{
  p().uninstall_dependencies(force_);
}

} // namespace zc
