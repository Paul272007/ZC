#pragma once

#include "../config/GConf.h"
#include "Command.h"

namespace zc
{

class Login : public Command
{
public:
  Login(bool force);
  int operator()() override;

private:
  GConf &gc_ = GConf::get();
};

} // namespace zc
