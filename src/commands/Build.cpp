#include "commands/Build.h"

#include <filesystem>
#include <string>

#include "commands/BuildCommand.h"
#include "commands/Command.h"
#include "config/GConf.h"
#include "helpers.h"
#include "pkgs/PkgType.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Build::Build(
  const CommandContext &c_ctx, const BuildContext &b_ctx, std::string target, bool clean, bool release,
  bool debug, bool run, const std::vector<std::string> &run_args
)
  : Command(c_ctx),
    BuildCommand(b_ctx),
    run_args_(run_args),
    target_(!target.empty() ? std::move(target) : "all"),
    run_(run),
    clean_(clean)
{
  mode_ = parse_mode<BuildMode>(
    {
      { BuildMode::release, release },
      { BuildMode::debug, debug },
    },
    BuildMode::automatic
  );
}

void Build::operator()()
{
  if (clean_)
    p().clean();

  p().build(mode_, false, jobs_, target_);

  if (p().pconf.type == PkgType::BIN && gc_.move_bin_to_current_path)
    if (fs::path binary = p().build_dir / p().pconf.target; fs::exists(binary))
      fs::rename(binary, fs::current_path() / p().pconf.target);

  if (run_)
    p().execute(run_args_);
}

} // namespace zc
