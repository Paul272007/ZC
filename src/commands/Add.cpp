#include "Add.h"

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
  : Command(force), static_(is_static), p_root_(get_project_root(p_root)), targets_(targets)
{
}

void Add::operator()()
{
  Project p(p_root_);
  for (const auto target : targets_)
    p.add_dependency(target, static_);
}

} // namespace zc
