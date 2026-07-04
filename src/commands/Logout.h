#pragma once

#include "Command.h"

namespace zc
{

class Logout : public Command
{
public:
  explicit Logout(const CommandContext &ctx);
  void operator()() override;
};

} // namespace zc
