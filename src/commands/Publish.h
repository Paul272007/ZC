#pragma once

#include "Command.h"

namespace zc
{

class Publish : public Command
{
public:
  Publish(bool force, const std::filesystem::path &p_root);

  int operator()() override;

private:
  const std::filesystem::path p_root_;
};

} // namespace zc
