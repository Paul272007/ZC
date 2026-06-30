#include "Use.h"

#include "commands/Command.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Use::Use(
  const bool force, const std::filesystem::path &p_root, const std::vector<std::string> &targets,
  const bool global
)
  : Command(force),
    global_(global),
    p_root_(!global ? get_project_root(p_root) : fs::current_path()),
    targets_(LocalTarget::parse(targets))
{
}

void Use::operator()()
{
  if (global_)
  {
    Registry &r = Registry::get();
    for (CAA[name, new_version] : targets_)
      r.set_default_version(name, new_version);
  }
  else
  {
    Project p(p_root_);
    for (CAA[name, new_version] : targets_)
      p.change_dependency_version(name, new_version);
  }
}

} // namespace zc
