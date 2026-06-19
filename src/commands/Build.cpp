#include "commands/Build.h"
#include "commands/Command.h"
#include "config/GConf.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Build::Build(const bool force, const std::filesystem::path &p_root, bool clean, bool release, bool debug)
    : Command(force), clean_(clean), p_root_(get_project_root(p_root))
{
  mode_ = parse_mode<BuildMode>({{BuildMode::release, release}, {BuildMode::debug, debug}}, BuildMode::debug);
}

void Build::operator()()
{
  Project p(p_root_);
  GConf gc(GConf::get());

  if (clean_)
    p.clean();

  p.build(mode_);

  if (p.pconf.type == BIN && gc.move_bin_to_current_path)
    if (fs::path binary = p.build_dir / p.pconf.target; fs::exists(binary))
      fs::rename(binary, fs::current_path() / p.pconf.target);
}

} // namespace zc
