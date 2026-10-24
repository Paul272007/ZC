#pragma once

#include "commands/Command.hpp"
#include "core/global-config/GlobalConfig.hpp"

namespace zc::app
{

class Logout final : public Command
{
public:
  explicit Logout(const CommandCtx &cmd_ctx) : Command(cmd_ctx, false) {}

  void execute() override
  {
    core::gc().setToken("");
    core::gc().setUsername("");
  }
};

} // namespace zc::app
