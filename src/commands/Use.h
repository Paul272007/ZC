#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "commands/ProjectCommand.h"
#include "pkgs/LocalTarget.h"

namespace zc
{

class Use : public ProjectCommand
{
public:
  Use(
    bool force, const std::filesystem::path &p_root, const std::vector<std::string> &targets, bool global
  );

  void operator()() override;

private:
  std::vector<LocalTarget> targets_;

  const bool global_;
};

} // namespace zc
