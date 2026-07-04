#pragma once

#include "commands/Command.h"
#include "Context.h"

namespace zc
{

class Publish : public Command
{
public:
  Publish(const CommandContext &ctx);

  void operator()() override;
};

} // namespace zc
