#pragma once

#include "../project/Project.h"
#include "Command.h"

namespace zc
{

class Build : public Command
{
public:
  Build(bool force, const std::filesystem::path &p_root, bool clean, bool release, bool debug);

  void operator()() override;

private:
  const bool clean_;
  const std::filesystem::path p_root_;
  BuildMode mode_;
};

} // namespace zc
