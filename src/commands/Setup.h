#pragma once

#include "Command.h"

namespace zc
{

class Setup : public Command
{
public:
  Setup(bool force, const std::filesystem::path &p_root);

  void operator()() override;

private:
  const std::filesystem::path p_root_;
};

} // namespace zc
