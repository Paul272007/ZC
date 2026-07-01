#pragma once

#include <string>

#include "Command.h"

namespace zc
{

class Config : public Command
{
public:
  Config(bool force, std::string key, std::string value);

  void operator()() override;

private:
  const std::string key_;
  const std::string value_;
};

} // namespace zc
