#pragma once

#include <string>
#include <vector>

#include "../pkgs/Registry.h"
#include "Command.h"

namespace zc
{

// TODO : Uninstall a particular version instead of the entire package each time

class Uninstall : public Command
{
public:
  Uninstall(bool force, std::vector<std::string> &targets);

  void operator()() override;

private:
  std::vector<std::string> targets_;
  Registry &reg_ = Registry::get();
};

} // namespace zc
