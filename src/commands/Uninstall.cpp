#include "Uninstall.h"

#include <filesystem>

#include "commands/ProjectCommand.h"
#include "helpers.h"
#include "pkgs/LocalTarget.h"

ZC_DEV_CONFIG

namespace zc
{

Uninstall::Uninstall(const bool force, const fs::path &p_root, const std::vector<std::string> &targets)
  : ProjectCommand(force, p_root, targets.empty()), targets_(LocalTarget::parse(targets))
{
}

void Uninstall::operator()()
{
  if (has_project())
    p().uninstall_dependencies();

  else
    for (const auto &target : targets_)
      if (target.version.is_empty())
        reg_.uninstall(target.name);
      else
        reg_.uninstall(target);
}

} // namespace zc
