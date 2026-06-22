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
  Registry &reg_ = Registry::get();

  const std::filesystem::path p_root_;

  Targets targets_;
};

} // namespace zc
