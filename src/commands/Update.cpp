#include "Update.h"

#include <filesystem>

#include "commands/Command.h"
#include "helpers.h"
#include "pkgs/RemoteTarget.h"
#include "project/Project.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

Update::Update(
  const bool force, const fs::path &p_root, fs::path path, const std::vector<std::string> &targets,
  const bool sync, const bool dont_use
)
  : Command(force),
    p_root_(get_project_root(p_root)),
    path_(std::move(path)),
    targets_(RemoteTarget::parse(targets)),
    sync_(sync),
    use_(!dont_use)
{
}

void Update::operator()()
{
  if (!path_.empty())
  {
    if (!targets_.empty())
      throw ZCException(
        ZCE_INCOMPATIBLE_FLAGS, "Cannot update from remote and local project at the same time"
      );

    Project p = reg_.update_from_path(path_, force_, use_);
    if (sync_)
      Project(p_root_).change_dependency_version(p.pconf.name, p.pconf.version);
  }
  else if (targets_.empty())
  {
    Project p(p_root_);
    p.update_dependencies(force_, use_); // --sync by default
  }
  else
  {
    for (auto &target : targets_)
      reg_.update_from_server(target, force_, use_);

    if (sync_)
    {
      Project p(p_root_); // target.version now contains the newly installed version :
      for (CAA[name, url, sha, new_version] : targets_)
        p.change_dependency_version(name, new_version);
    }
  }
}

} // namespace zc
