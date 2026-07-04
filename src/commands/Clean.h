#pragma once

#include "commands/Command.h"
#include "Context.h"

namespace zc
{

class Clean : public Command
{
public:
  Clean(const CommandContext &ctx);

  void operator()() override;
};

} // namespace zc
