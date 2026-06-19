#pragma once

#include "Command.h"

namespace zc
{

class Build : public Command
{
public:
  Build(bool force, const std::filesystem::path &p_root, bool clean, bool release);

  void operator()() override;

private:
  const bool clean_;
  const bool release_;
  const std::filesystem::path p_root_;
};

} // namespace zc
