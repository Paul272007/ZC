#pragma once

#include "Command.h"

namespace zc
{

class Logout : public Command
{
public:
  explicit Logout(bool force);
  void operator()() override;
};

} // namespace zc
