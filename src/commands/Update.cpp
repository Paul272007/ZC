#include "Update.h"

#include <filesystem>

#include "commands/ProjectCommand.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "pkgs/RemoteTarget.h"
#include "project/Project.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

Update::Update(
  const bool force, const fs::path &p_root, fs::path path, const vector<string> &targets, const bool sync,
  const bool dont_use, const bool save_path
)
  : ProjectCommand(force, p_root, (targets.empty() && path.empty()) || sync),
    path_(std::move(path)),
    targets_(parse_targets(targets)),
    use_(!dont_use),
    sync_(sync),
    save_path_(save_path)
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
  for (CAA[name, version] : targets_)
  {
    const Pkg &pkg = reg_.get_pkg(name);
    if (pkg.origin == "local")
    {
      if (pkg.path.empty())
        throw ZCException(
          ZCE_NOT_FOUND,
          "Local package '" + name + "' doesn't have a saved path. Please use --path to update it."
        );

      if (!fs::exists(pkg.path))
        throw ZCException(ZCE_NOT_FOUND, "The package path '" + pkg.path + "' doesn't exist.");

      Project project = reg_.update_from_path(pkg.path, force_, use_);
      if (sync_)
        p().change_dependency_version(project.pconf.name, project.pconf.version);
    }
    else
    {
      auto remote_target = RemoteTarget::get_target({ name, version });

      reg_.update_from_server(remote_target, force_, use_);
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

  Project project = reg_.update_from_path(path_, force_, use_, save_path_);
  if (sync_)
    p().change_dependency_version(project.pconf.name, project.pconf.version);
}

void Update::update_dependencies()
{
  p().update_dependencies(force_, use_); // Changes version in project config by default
}

// sync_project method removed as it's now handled directly inside update_targets

} // namespace zc
