#pragma once

#include "AppCommand.hpp"
#include "core/global-config/GlobalConfig.hpp"

namespace zc::app
{

class Logout final : public AppCommand<"logout", "Log out of your account">
{
public:
  Logout() = default;

  void run() override
  {
    core::gc().setToken("");
    core::gc().setUsername("");
  }
};

} // namespace zc::app
