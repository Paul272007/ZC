//
// Created by paul on 14/06/2026.
//

#pragma once

#include "Command.h"

namespace zc
{

class Clean : public Command
{
public:
  Clean(bool force, const std::filesystem::path &p_root);

  void operator()() override;

private:
  const std::filesystem::path p_root_;
};

} // namespace zc
