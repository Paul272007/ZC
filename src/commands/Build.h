#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "commands/ProjectCommand.h"
#include "project/Project.h"

namespace zc
{

class Build : public ProjectCommand
{
public:
  Build(
    bool force, const std::filesystem::path &p_root, bool clean, bool release, bool debug, bool run,
    const std::vector<std::string> &run_args, bool jobs_given, int input_jobs
  );

  void operator()() override;

private:
  const std::vector<std::string> run_args_;

  const bool   run_;
  const bool   clean_;
  const size_t jobs_;
  BuildMode    mode_;
};

} // namespace zc
