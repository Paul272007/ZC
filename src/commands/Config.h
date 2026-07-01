#pragma once

#include <string>

#include "Command.h"
#include "config/GConf.h"

namespace zc
{

class Config : public Command
{
public:
  Config(bool force, std::string key, std::string value);

  void operator()() override;

private:
  GConf &gc_ = GConf::get();

  const std::string key_;
  const std::string value_;
};

} // namespace zc
