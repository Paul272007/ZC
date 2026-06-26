#include "commands/Build.h"

#include <filesystem>

#include "commands/Command.h"
#include "config/GConf.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Build::Build(
  const bool force, const std::filesystem::path &p_root, bool clean, bool release, bool debug, bool run,
  const std::vector<std::string> &run_args
)
  : Command(force), clean_(clean), p_root_(get_project_root(p_root)), run_(run), run_args_(run_args)
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
  Project p(p_root_);

  if (clean_)
    p.clean();

  p.build(mode_);

  if (p.pconf.type == BIN && gc_.move_bin_to_current_path)
    if (fs::path binary = p.build_dir / p.pconf.target; fs::exists(binary))
      fs::rename(binary, fs::current_path() / p.pconf.target);

  if (run_)
  {
    string exec_cmd = escape_shell_arg(
      fs::absolute(
        (p.pconf.type == BIN && gc_.move_bin_to_current_path) ? fs::current_path() / p.pconf.target
                                                              : p.build_dir / p.pconf.target
      )
    );
    for (const auto &arg : run_args_)
      exec_cmd += " " + escape_shell_arg(arg);
    const int run_res = system(exec_cmd.c_str());
    if (run_res != 0)
      throw ZCException(ZCE_RUNTIME_ERROR, "Program exited with code " + to_string(run_res));
  }
}

} // namespace zc
