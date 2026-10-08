#pragma once

#include "commands/Command.h"
#include "Context.h"

namespace zc
{

class Components : public Command
{
protected:
  Components(const ComponentsContext &ctx) : Command(ctx.c_ctx) {}

  std::vector<Component> components_;
};

} // namespace zc
