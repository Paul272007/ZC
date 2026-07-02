#include "Update.h"

#include <filesystem>

#include "commands/ProjectCommand.h"
#include "helpers.h"
#include "pkgs/RemoteTarget.h"
#include "project/Project.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

Update::Update(
  const bool force, const fs::path &p_root, fs::path path, const vector<string> &targets, const bool sync,
  const bool dont_use
)
  : ProjectCommand(force, p_root, (targets.empty() && path.empty()) || sync),
    path_(std::move(path)),
    targets_(RemoteTarget::parse(targets)),
    use_(!dont_use),
    sync_(sync)
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
    reg_.update_from_server(target, force_, use_);
  sync_project();
}

void Update::update_from_path()
{
  if (!targets_.empty())
    throw ZCException(
      ZCE_INCOMPATIBLE_FLAGS, "Cannot update from remote and local project at the same time"
    );

  Project project = reg_.update_from_path(path_, force_, use_);
  if (sync_)
    p().change_dependency_version(project.pconf.name, project.pconf.version);
}

void Update::update_dependencies()
{
  p().update_dependencies(force_, use_); // Changes version in project config by default
}

void Update::sync_project()
{
  if (sync_)
    for (CAA[name, url, sha, new_version] : targets_)
      p().change_dependency_version(name, new_version);
}

} // namespace zc
