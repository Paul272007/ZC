#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "commands/ProjectCommand.h"
#include "pkgs/Registry.h"
#include "pkgs/RemoteTarget.h"

namespace zc
{

class Install : public ProjectCommand
{
public:
  Install(
    bool force, const std::filesystem::path &p_root, std::filesystem::path path,
    const std::vector<std::string> &targets, bool sync, bool is_std
  );

  void operator()() override;

private:
  Registry &reg_ = Registry::get();

  const std::filesystem::path path_;
  std::vector<RemoteTarget>   targets_;

  const bool std_;
  const bool sync_;

  void install_from_path();
  void install_dependencies();
  void install_targets();
  void sync_project();
};

} // namespace zc
