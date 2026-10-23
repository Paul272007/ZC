#pragma once
#include "framework/util/types.hpp"

namespace zc::app
{

struct InstallCommandCtx {
  /// @brief Whether to add installed target(s) as dependencies to current project
  bool sync = false;
  /// @brief Path to local project to install
  stdfs::path target_path;
  /// @brief Targets to install
  std::vector<std::string> targets;
};

} // namespace zc::app
