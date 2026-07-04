#include "Add.h"

#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "pkgs/PkgType.h"
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
  if (p().pconf.type == PkgType::COMPOSE)
    throw ZCException(ZCE_TYPE_ERROR, "Cannot add dependency to package of type COMPOSE");

  for (CAA target : targets_)
    p().add_dependency(LocalTarget::get_target(target), static_);
}

} // namespace zc
