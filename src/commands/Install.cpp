#include "Install.h"
#include "commands/Command.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "pkgs/Network.h"
#include "project/Project.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

Install::Install(bool force, const std::string &path, std::vector<std::string> &targets)
    : Command(force), path_(path), targets_(parse_targets(targets))
{
}

void Install::operator()()
{
  if (!path_.empty())
  {
    if (!targets_.empty())
      throw ZCException(
          ZCE_INCOMPATIBLE_FLAGS, "Cannot install from remote and from local project at the same time"
      );

    reg_.install_from_path(path_, force_);
    return;
  }
  if (targets_.empty())
  {
    Project p;
    p.install_dependencies();
    return;
  }

  json index = Network::get().get_index();
  for (const auto &target : targets_) reg_.install_from_server(target.name, target.version, index, force_);
}

} // namespace zc
