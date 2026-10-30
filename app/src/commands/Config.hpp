#pragma once

#include <string_view>

#include "commands/AppCommand.hpp"
#include "core/global-config/GlobalConfig.hpp"
#include "framework/cli/Args.hpp"

namespace zc::app
{

class Config final : public AppCommand<"config", "Manage global configuration">
{
  ZC_REQUIRED_POSITIONAL(key, std::string_view, "Configuration key to modify");
  ZC_POSITIONAL(value, std::string_view, "Value given to the key");

public:
  Config() = default;

  auto options() { return std::tie(key_, value_); }

  void run() override
  {
    auto conf_res = core::gc().set(*key_, *value_);
    if (!conf_res)
      throw_or_terminate(conf_res.error());
  }
};

} // namespace zc::app
