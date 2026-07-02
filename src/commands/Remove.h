#pragma once

#include "ProjectCommand.h"

namespace zc
{

class Remove : public ProjectCommand
{
public:
  Remove(bool force, const std::filesystem::path &p_root, const std::vector<std::string> &targets);

  void operator()() override;

private:
  const std::vector<std::string> targets_;
};

} // namespace zc
