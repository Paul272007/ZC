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
    const std::vector<std::string> &targets, bool sync, bool dont_use
  );

  void operator()() override;

private:
  Registry &reg_ = Registry::get();

  const std::filesystem::path path_;
  std::vector<RemoteTarget>   targets_;

  const bool use_;
  const bool sync_;

  void update_from_path();
  void update_dependencies();
  void update_targets();
  void sync_project();
};

} // namespace zc
