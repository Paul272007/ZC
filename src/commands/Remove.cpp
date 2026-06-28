#include "Remove.h"

#include "commands/Command.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Remove::Remove(
  const bool force, const std::filesystem::path &p_root, const std::vector<std::string> &targets
)
  : Command(force), p_root_(get_project_root(p_root)), targets_(targets)
{
}

void Remove::operator()()
{
  Project p(p_root_);

  for (CAA target : targets_)
    p.remove_dependency(target);
}

} // namespace zc
