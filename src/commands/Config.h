#pragma once

#include <string>

#include "commands/Command.h"
#include "config/GConf.h"
#include "Context.h"

namespace zc
{

class Config : public Command
{
public:
  Config(const CommandContext &ctx, std::string key, std::string value);

  void operator()() override;

private:
  GConf &gc_ = GConf::get();

  const std::string key_;
  const std::string value_;
};

} // namespace zc
