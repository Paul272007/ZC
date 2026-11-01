#pragma once

#include <filesystem>
#include <string_view>

#include "commands/AppCommand.hpp"
#include "framework/cli/Args.hpp"
#include "framework/cli/FixedString.hpp"

namespace zc::app
{

template<cli::FixedString Name, cli::FixedString Desc>
class PackageCommand : public AppCommand<Name, Desc>
{
protected:
  ZC_LIST_POSITIONAL(packages, std::string, "Packages to install");
  ZC_OPTION(
    local, stdfs::path, 'l', "Target local project instead of remote projects", stdfs::current_path()
  );
  ZC_OPTION(target, std::string_view, 't', "Project to sync changes with", {});
  ZC_FLAG(sync, 's', "Sync changes with target dependencies");

  // unused for the moment
  [[nodiscard]] auto options_parent() { return std::tie(packages_, local_, target_, sync_); }

  [[nodiscard]] bool actOnProject() const noexcept { return !packages_ && !local_; }

public:
  [[nodiscard]] bool requiresProject() const override { return actOnProject() || sync_; }
};

} // namespace zc::app
