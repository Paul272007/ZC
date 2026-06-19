#pragma once

#include "Command.h"

namespace zc
{

class Create : public Command
{
public:
  Create(bool force);

  void operator()() override;
};

} // namespace zc
