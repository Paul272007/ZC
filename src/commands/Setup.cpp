#include "Setup.h"

#include "commands/Command.h"
#include "Context.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Setup::Setup(const CommandContext &ctx, const bool release, const bool debug)
  : Command(ctx)
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
  p().generate_build_config(mode_);
}

} // namespace zc
