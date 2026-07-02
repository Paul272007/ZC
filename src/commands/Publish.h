#pragma once

#include "commands/ProjectCommand.h"

namespace zc
{

class Publish : public ProjectCommand
{
public:
  Publish(bool force, const std::filesystem::path &p_root);

  void operator()() override;
};

} // namespace zc
