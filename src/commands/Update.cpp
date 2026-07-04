#include "Update.h"

#include <filesystem>

#include "commands/BuildCommand.h"
#include "commands/InstallCommand.h"
#include "Context.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "pkgs/RemoteTarget.h"
#include "project/Project.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

Update::Update(
  CommandContext &c_ctx, BuildContext &b_ctx, InstallContext &i_ctx, bool dont_use, bool save_path
)
  : BuildCommand(b_ctx), InstallCommand(c_ctx, i_ctx), use_(!dont_use), save_path_(save_path)
{
}

void Update::operator()()
{
  if (has_project())
    update_dependencies();
  elif (!path_.empty())
    update_from_path();
  else
    update_targets();
}

void Update::update_targets()
{
  for (CAA target : targets_)
  {
    const Pkg &pkg = reg_.get_pkg(target.first);
    if (pkg.origin == "local")
    {
      if (pkg.path.empty())
        throw ZCException(
          ZCE_NOT_FOUND,
          "Local package '" + target.first + "' doesn't have a saved path. Please use --path to update it."
        );

      if (!fs::exists(pkg.path))
        throw ZCException(ZCE_NOT_FOUND, "The package path '" + pkg.path + "' doesn't exist.");

      Project project = reg_.update_from_path(pkg.path, force_, use_, false, jobs_);
      if (sync_)
        p().change_dependency_version(project.pconf.name, project.pconf.version);
    }
    else
    {
      auto remote_target = RemoteTarget::get_target(target);

      reg_.update_from_server(remote_target, force_, use_, jobs_);
      if (sync_)
        p().change_dependency_version(remote_target.name, remote_target.version);
    }
  }
}

void Update::update_from_path()
{
  if (!targets_.empty())
    throw ZCException(
      ZCE_INCOMPATIBLE_FLAGS, "Cannot update from remote and local project at the same time"
    );

  Project project = reg_.update_from_path(path_, force_, use_, save_path_, jobs_);
  if (sync_)
    p().change_dependency_version(project.pconf.name, project.pconf.version);
}

void Update::update_dependencies()
{
  p().update_dependencies(force_, use_); // --sync by default
}

} // namespace zc
