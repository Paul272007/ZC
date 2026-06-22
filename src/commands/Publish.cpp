#include "commands/Publish.h"

#include "commands/Command.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Publish::Publish(const bool force, const std::filesystem::path &p_root)
  : Command(force), p_root_(get_project_root(p_root))
{
}

void Publish::operator()()
{
  Project(p_root_).publish();
}

} // namespace zc
