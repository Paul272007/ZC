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

  // Deduce target if we are inside a component
  if (target_ == "all" && p().pconf.type == PkgType::COMPOSE)
  {
    std::error_code ec;
    fs::path        current = fs::current_path(ec);
    fs::path        root    = p().root_dir;

    // Check if current path is inside root_dir
    auto rel = fs::relative(current, root, ec);
    if (!ec && !rel.empty() && rel != ".")
    {
      // Extract the first directory name which is the component name
      std::string comp_name = *rel.begin();
      if (p().pconf.components.end() !=
          std::find(p().pconf.components.begin(), p().pconf.components.end(), comp_name))
      {
        target_ = comp_name;
        if_.debug("Auto-detected component target: " + target_);
      }
    }
  }

  p().build(mode_, false, jobs_, target_);

  if (p().pconf.type == PkgType::BIN && gc_.move_bin_to_current_path)
    if (fs::path binary = p().build_dir / p().pconf.target; fs::exists(binary))
      fs::rename(binary, fs::current_path() / p().pconf.target);

  if (run_)
    p().execute(run_args_);
}

} // namespace zc
