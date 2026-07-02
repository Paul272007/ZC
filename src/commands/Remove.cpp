#include "Remove.h"

#include "commands/ProjectCommand.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Remove::Remove(
  const bool force, const std::filesystem::path &p_root, const std::vector<std::string> &targets
)
  : ProjectCommand(force, p_root), targets_(targets)
{
}

void Remove::operator()()
{
  for (CAA target : targets_)
    p().remove_dependency(target);
}

} // namespace zc
