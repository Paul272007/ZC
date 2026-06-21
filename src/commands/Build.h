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
  const bool clean_;
  const std::filesystem::path p_root_;
  BuildMode mode_;
  const bool run_;
  const std::vector<std::string> run_args_;
};

} // namespace zc
