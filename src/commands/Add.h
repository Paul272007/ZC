#pragma once

#include "Command.h"

namespace zc
{

class Add : public Command
{
public:
  Add(
    bool force, const std::filesystem::path &p_root, const std::vector<std::string> &targets, bool is_static
  );

  void operator()() override;

private:
  const std::vector<std::string> targets_;
  const std::filesystem::path    p_root_;

  const bool static_;
};

} // namespace zc
