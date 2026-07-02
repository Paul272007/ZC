#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "pkgs/Registry.h"
#include "ProjectCommand.h"

namespace zc
{

class Uninstall : public ProjectCommand
{
public:
  Uninstall(bool force, const std::filesystem::path &p_root, const std::vector<std::string> &targets);

  void operator()() override;

private:
  Registry &reg_ = Registry::get();

  std::vector<LocalTarget> targets_;
};

} // namespace zc
