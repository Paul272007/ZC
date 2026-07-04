#pragma once

#include "commands/InstallCommand.h"
#include "Context.h"

namespace zc
{

class Add : public InstallCommand
{
public:
  Add(CommandContext &c_ctx, InstallContext &i_ctx, bool is_static);

  void operator()() override;

private:
  const bool static_;
};

} // namespace zc
