#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "Command.h"
#include "pkgs/LocalTarget.h"

namespace zc
{

class Use : public Command
{
public:
  Use(
    bool force, const std::filesystem::path &p_root, const std::vector<std::string> &targets, bool global
  );

  void operator()() override;

private:
  const bool                  global_;
  const std::filesystem::path p_root_;

  std::vector<LocalTarget> targets_;
};

} // namespace zc
