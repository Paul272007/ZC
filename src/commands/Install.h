#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../pkgs/Registry.h"
#include "../pkgs/RemoteTarget.h"
#include "Command.h"

namespace zc
{

class Install : public Command
{
public:
  Install(
    bool force, const std::filesystem::path &p_root, std::filesystem::path path,
    const std::vector<std::string> &targets, bool is_std
  );

  void operator()() override;

private:
  Registry &reg_ = Registry::get();

  const std::filesystem::path p_root_;
  const std::filesystem::path path_;

  std::vector<RemoteTarget> targets_;

  const bool std_;

  void install_from_path() const;
  void install_dependencies() const;
  void install_targets();
};

} // namespace zc
