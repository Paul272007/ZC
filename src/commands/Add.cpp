#include "Add.h"

#include "helpers.h"
#include "pkgs/Registry.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Add::Add(CommandContext &c_ctx, InstallContext &i_ctx, bool is_static)
  : InstallCommand(c_ctx, i_ctx), static_(is_static)
{
}

void Add::operator()()
{
  for (CAA target : targets_)
    p().add_dependency(LocalTarget::get_target(target), static_);
}

} // namespace zc
