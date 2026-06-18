//
// Created by paul on 15/06/2026.
//

#pragma once

#include "Command.h"

namespace zc
{

class Remove : public Command
{
public:
  Remove(bool force, const std::filesystem::path &p_root, const std::vector<std::string> &targets);

  int operator()() override;

private:
  const std::filesystem::path p_root_;
  const std::vector<std::string> targets_;
};

} // namespace zc
