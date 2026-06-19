//
// Created by paul on 14/06/2026.
//

#pragma once

#include "Command.h"

namespace zc
{

class Add : public Command
{
public:
  Add(bool force, const std::filesystem::path &p_root, const std::vector<std::string> &targets);

  void operator()() override;

private:
  const std::filesystem::path p_root_;
  const std::vector<std::string> targets_;
};

} // namespace zc
