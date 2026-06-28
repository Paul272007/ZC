#pragma once

#include "Command.h"

namespace zc
{

class Login : public Command
{
public:
  explicit Login(bool force);
  void operator()() override;
};

} // namespace zc
