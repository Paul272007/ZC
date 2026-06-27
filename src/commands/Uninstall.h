#pragma once

#include <string>
#include <vector>

#include "../pkgs/Registry.h"
#include "Command.h"

namespace zc
{

// TODO: Uninstall a particular version instead of the entire package each time

class Uninstall : public Command
{
public:
  Uninstall(bool force, std::vector<std::string> &targets);

  void operator()() override;

private:
  Registry &reg_ = Registry::get();

  std::vector<std::string> targets_;
};

} // namespace zc
