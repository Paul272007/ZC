#pragma once

#include "commands/ProjectCommand.h"
#include "project/Project.h"

namespace zc
{

class Setup : public ProjectCommand
{
public:
  Setup(bool force, const std::filesystem::path &p_root, bool release, bool debug);

  void operator()() override;

private:
  BuildMode mode_;
};

} // namespace zc
