#pragma once

#include "commands/InstallCommand.h"
#include "Context.h"

namespace zc
{

class Remove : public InstallCommand
{
public:
  Remove(CommandContext &c_ctx, InstallContext &i_ctx);

  void operator()() override;
};

} // namespace zc
