#include "Install.h"

#include <filesystem>

#include "commands/ProjectCommand.h"
#include "helpers.h"
#include "pkgs/RemoteTarget.h"
#include "project/Project.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

Install::Install(
  const bool force, const fs::path &p_root, fs::path path, const vector<string> &targets, const bool sync,
  const bool is_std
)
  : ProjectCommand(force, p_root, (targets.empty() && path.empty()) || sync),
    path_(std::move(path)),
    std_(is_std),
    sync_(sync)
{
  if (std_)
    for (CAA t : targets)
      targets_.push_back({ .name = t, .url = "", .sha256 = "", .version = { 0, 0, 0 } });
  else
    targets_ = RemoteTarget::parse(targets);
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

  Project project = reg_.install_from_path(path_, force_);
  if (sync_)
    p().add_dependency({ .name = project.pconf.name, .version = project.pconf.version });
}

void Install::install_dependencies()
{
  p().install_dependencies();
}

void Install::install_targets()
{
  if (std_)
  {
    if (!has_pkg_config())
      throw ZCException(ZCE_NOT_FOUND, "Command 'pkg-config' is required to install standard packages");
    for (CAA[name, url, sha, version] : targets_)
      reg_.install_std(name);
  }
  else
  {
    for (CAA target : targets_)
      reg_.install_from_server(target, force_);
  }
}

void Install::sync_project()
{
  if (sync_)
    for (CAA[name, url, sha, version] : targets_)
      p().add_dependency({ .name = name, .version = version });
}

} // namespace zc
