#pragma once

#include "commands/ProjectCommand.h"

namespace zc
{

class Clean : public ProjectCommand
{
public:
  Clean(bool force, const std::filesystem::path &p_root);

  void operator()() override;
};

} // namespace zc
