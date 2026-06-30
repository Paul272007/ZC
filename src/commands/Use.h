#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../pkgs/Registry.h"
#include "Command.h"

namespace zc
{

class Use : public Command
{
public:
  Use(bool force, const std::filesystem::path &p_root, const std::vector<std::string> &targets);

  void operator()() override;

private:
  Registry &reg_ = Registry::get();

  const std::filesystem::path p_root_;

  std::vector<LocalTarget> targets_;
};

} // namespace zc
