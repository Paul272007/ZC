#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../helpers.h"
#include "../pkgs/Registry.h"
#include "Command.h"

namespace zc
{

class Use : public Command
{
public:
  Use(bool force, const std::filesystem::path &p_root, std::vector<std::string> &targets);

  void operator()() override;

private:
  const std::filesystem::path p_root_;
  Targets targets_;
  Registry &reg_ = Registry::get();
};

} // namespace zc
