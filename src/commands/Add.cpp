#include "Add.h"

#include <string>
#include <vector>

#include "commands/Command.h"
#include "helpers.h"
#include "pkgs/Registry.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Add::Add(
  const bool force, const std::filesystem::path &p_root, const std::vector<std::string> &targets,
  const bool is_static
)
  : Command(force), p_root_(get_project_root(p_root)), targets_(parse_targets(targets)), static_(is_static)
{
}

void Add::operator()()
{
  Project p(p_root_);
  for (CAA target : targets_)
    p.add_dependency(target, static_);
}

} // namespace zc
