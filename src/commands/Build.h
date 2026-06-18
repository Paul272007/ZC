//
// Created by paul on 14/06/2026.
//

#pragma once

#include "Command.h"

namespace zc
{

class Build : public Command
{
public:
  Build(bool force, const std::filesystem::path &p_root, bool clean, bool release);

  int operator()() override;

private:
  const bool clean_;
  const bool release_;
  const std::filesystem::path p_root_;
};

} // namespace zc
