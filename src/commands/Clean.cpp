#include "commands/Clean.h"
#include "commands/Command.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Clean::Clean(const bool force, const std::filesystem::path &p_root)
    : Command(force), p_root_(get_project_root(p_root))
{
}

int Clean::operator()()
{
  Project p(p_root_);
  p.clean();
  return 0;
}

} // namespace zc
