#pragma once

#include <string>
#include <vector>

#include "../project/Project.h"
#include "Command.h"

namespace zc
{

class Build : public Command
{
public:
  Build(
    bool force, const std::filesystem::path &p_root, bool clean, bool release, bool debug, bool run = false,
    const std::vector<std::string> &run_args = {}
  );

  void operator()() override;

private:
  const std::filesystem::path    p_root_;
  const std::vector<std::string> run_args_;

  const bool run_;
  const bool clean_;
  BuildMode  mode_;
};

} // namespace zc
