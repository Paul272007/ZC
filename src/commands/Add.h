#pragma once

#include "commands/ProjectCommand.h"
#include "pkgs/LocalTarget.h"

namespace zc
{

class Add : public ProjectCommand
{
public:
  Add(
    bool force, const std::filesystem::path &p_root, const std::vector<std::string> &targets, bool is_static
  );

  void operator()() override;

private:
  const std::vector<LocalTarget> targets_;

  const bool static_;
};

} // namespace zc
