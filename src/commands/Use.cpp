#include "Use.h"

#include "commands/ProjectCommand.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Use::Use(
  const bool force, const std::filesystem::path &p_root, const std::vector<std::string> &targets,
  const bool global
)
  : ProjectCommand(force, p_root, !global), targets_(LocalTarget::parse(targets)), global_(global)
{
}

void Use::operator()()
{
  if (global_)
    for (CAA[name, new_version] : targets_)
      rg().set_default_version(name, new_version);
  else
    for (CAA[name, new_version] : targets_)
      p().change_dependency_version(name, new_version);
}

} // namespace zc
