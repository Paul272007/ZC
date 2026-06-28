#include "Setup.h"

#include "commands/Command.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Setup::Setup(const bool force, const std::filesystem::path &p_root, const bool release, const bool debug)
  : Command(force), p_root_(get_project_root(p_root))
{
  mode_ = parse_mode<BuildMode>(
    {
      { BuildMode::release, release },
      { BuildMode::debug, debug },
    },
    BuildMode::automatic
  );
}

void Setup::operator()()
{
  Project p(p_root_);
  p.generate_build_config(mode_);
}

} // namespace zc
