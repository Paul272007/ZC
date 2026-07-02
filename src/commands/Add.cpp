#include "Add.h"

#include <string>
#include <vector>

#include "helpers.h"
#include "pkgs/LocalTarget.h"
#include "pkgs/Registry.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Add::Add(
  const bool force, const std::filesystem::path &p_root, const std::vector<std::string> &targets,
  const bool is_static
)
  : ProjectCommand(force, p_root), targets_(LocalTarget::parse(targets)), static_(is_static)
{
}

void Add::operator()()
{
  for (CAA target : targets_)
    p().add_dependency(target, static_);
}

} // namespace zc
