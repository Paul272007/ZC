#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "commands/ProjectCommand.h"
#include "pkgs/Registry.h"

namespace zc
{

class Update : public ProjectCommand
{
public:
  Update(
    bool force, const std::filesystem::path &p_root, std::filesystem::path path,
    const std::vector<std::string> &targets, bool sync, bool dont_use, bool save_path
  );

  void operator()() override;

private:
  Registry &reg_ = Registry::get();

  const std::filesystem::path path_;
  std::vector<Target> targets_;

  const bool use_;
  const bool sync_;
  const bool save_path_;

  void update_from_path();
  void update_dependencies();
  void update_targets();
};

} // namespace zc
