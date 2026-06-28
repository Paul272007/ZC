#pragma once

#include "../project/Project.h"
#include "Command.h"

namespace zc
{

class Setup : public Command
{
public:
  Setup(bool force, const std::filesystem::path &p_root, bool release, bool debug);

  void operator()() override;

private:
  const std::filesystem::path p_root_;

  BuildMode mode_;
};

} // namespace zc
