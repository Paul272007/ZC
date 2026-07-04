#pragma once

#include <string>
#include <vector>

#include "commands/BuildCommand.h"
#include "commands/Command.h"
#include "Context.h"
#include "project/Project.h"

namespace zc
{

class Build : public Command, public BuildCommand
{
public:
  Build(
    const CommandContext &c_ctx, const BuildContext &b_ctx, std::string target, bool clean, bool release,
    bool debug, bool run, const std::vector<std::string> &run_args
  );

  void operator()() override;

private:
  const std::vector<std::string> run_args_;
  const std::string              target_;
  const bool                     run_;
  const bool                     clean_;
  BuildMode                      mode_;
};

} // namespace zc
