#pragma once

#include "Command.hpp"
#include "core/global-config/GlobalConfig.hpp"

namespace zc::app
{

class Config final : public Command
{
  std::string_view key_;
  std::string_view value_;

public:
  explicit Config(const CommandCtx &ctx, const std::string_view key, const std::string_view value)
    : Command(ctx, false), key_(key), value_(value)
  {
  }

  void execute() override
  {
    auto conf_res = core::gc().set(key_, value_);
    if (!conf_res)
      throw_or_terminate(conf_res.error());
  }
};

} // namespace zc::app
