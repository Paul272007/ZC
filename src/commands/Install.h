#pragma once

#include <filesystem>
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
  Install(
      bool force, const std::filesystem::path &p_root, const std::filesystem::path &path,
      std::vector<std::string> &targets
  );

  void operator()() override;

private:
  const std::filesystem::path p_root_;
  const std::filesystem::path path_;
  Targets targets_;
  Registry &reg_ = Registry::get();
};

} // namespace zc
