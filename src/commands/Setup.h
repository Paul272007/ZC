#pragma once

#include "commands/Command.h"
#include "Context.h"
#include "project/Project.h"

namespace zc
{

class Setup : public Command
{
public:
  Setup(const CommandContext &ctx, bool release, bool debug);

  void operator()() override;

private:
  BuildMode mode_;
};

} // namespace zc
