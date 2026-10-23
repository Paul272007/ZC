#pragma once

#include "context/InstallCommandCtx.hpp"
#include "framework/Version.hpp"
#include "util/helpers.hpp"

namespace zc::app
{

class InstallCommand
{
protected:
  /// @brief Whether to add installed target(s) as dependencies to current project
  const bool sync_;
  /// @brief Path to local project to install
  const stdfs::path &target_path_;
  /// @brief Targets to install
  const std::map<std::string, Version, std::less<>> targets_;

  explicit InstallCommand(InstallCommandCtx &ctx)
    : sync_(ctx.sync), target_path_(std::move(ctx.target_path)), targets_(parse_targets(ctx.targets))
  {
  }

public:
  InstallCommand(const InstallCommand &)            = delete;
  InstallCommand(InstallCommand &&)                 = delete;
  InstallCommand &operator=(const InstallCommand &) = delete;
  InstallCommand &operator=(InstallCommand &&)      = delete;
  ~InstallCommand()                                 = default;
};

} // namespace zc::app
