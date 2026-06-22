#pragma once

#include "Command.h"

namespace zc
{

class Logout : public Command
{
public:
  Logout(bool force);
  void operator()() override;
};

} // namespace zc
