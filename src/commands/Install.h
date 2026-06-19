#pragma once

#include <string>
#include <vector>

#include "../helpers.h"
#include "../pkgs/Registry.h"
#include "Command.h"

namespace zc
{

class Install : public Command
{
public:
  Install(bool force, const std::string &path, std::vector<std::string> &targets);

  void operator()() override;

private:
  const std::string path_;
  const Targets targets_;
  Registry &reg_ = Registry::get();
};

} // namespace zc
