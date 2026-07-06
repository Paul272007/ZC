#include "Remove.h"

#include "commands/InstallCommand.h"
#include "Context.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Remove::Remove(CommandContext &c_ctx, InstallContext &i_ctx) : InstallCommand(c_ctx, i_ctx, true) {}

void Remove::operator()()
{
  for (CAA target : targets_)
    p().remove_dependency(target.first); // TODO: implement version checking and all
}

} // namespace zc
