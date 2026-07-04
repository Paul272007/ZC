#pragma once

#include "Command.h"

namespace zc
{

class Login : public Command
{
public:
  explicit Login(const CommandContext &ctx);
  void operator()() override;
};

} // namespace zc
