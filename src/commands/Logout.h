#pragma once

#include "../config/GConf.h"
#include "Command.h"

namespace zc
{

class Logout : public Command
{
public:
  Logout(bool force);
  int operator()() override;

private:
  GConf &gc_ = GConf::get();
};

} // namespace zc
