#pragma once

#include <string>
#include <vector>

#include "commands/Command.h"
#include "Context.h"
#include "pkgs/LocalTarget.h"

namespace zc
{

class Use : public Command // TODO: inherit from InstallCommand
{
public:
  Use(const CommandContext &ctx, const std::vector<std::string> &targets, bool global);

  void operator()() override;

private:
  std::vector<LocalTarget> targets_;

  const bool global_;
};

} // namespace zc
