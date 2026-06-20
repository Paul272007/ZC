#include "Use.h"
#include "commands/Command.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Use::Use(bool force, const std::filesystem::path &p_root, std::vector<std::string> &targets)
    : Command(force), p_root_(get_project_root(p_root)), targets_(parse_targets(targets))
{
}

void Use::operator()()
{
  Project p(p_root_);
  for (const auto &target : targets_) p.change_dependency_version(target.name, target.version);
}

} // namespace zc
