#pragma once

#include "Command.h"

namespace zc
{

class Create : public Command
{
public:
  Create(bool force);

  int operator()() override;
};

} // namespace zc
