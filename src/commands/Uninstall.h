#pragma once

#include "commands/InstallCommand.h"
#include "Context.h"

namespace zc
{

class Uninstall : public InstallCommand
{
public:
  Uninstall(CommandContext &c_ctx, InstallContext &i_ctx);

  void operator()() override;

private:
  void uninstall_from_path();
  void uninstall_dependencies();
  void uninstall_targets();
};

} // namespace zc
