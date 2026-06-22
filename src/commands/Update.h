#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../helpers.h"
#include "../pkgs/Registry.h"
#include "Command.h"

namespace zc
{

class Update : public Command
{
public:
  Update(
    bool force, const std::filesystem::path &p_root, const std::filesystem::path &path,
    std::vector<std::string> &targets, bool sync
  );

  void operator()() override;

private:
  Registry &reg_ = Registry::get();

  const std::filesystem::path p_root_;
  const std::filesystem::path path_;

  Targets    targets_;
  const bool sync_;
};

} // namespace zc
