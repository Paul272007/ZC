#include "Setup.h"

#include "commands/Command.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Setup::Setup(bool force, const std::filesystem::path &p_root)
  : Command(force), p_root_(get_project_root(p_root))
{
}

void Setup::operator()()
{
  Project p(p_root_);
  p.generate_build_config();
}

} // namespace zc
