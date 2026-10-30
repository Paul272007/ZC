#pragma once

#include <filesystem>
#include <string>

#include "commands/AppCommand.hpp"
#include "core/global-config/GlobalConfig.hpp"
#include "core/registry/Registry.hpp"
#include "framework/cli/Args.hpp"
#include "framework/util/types.hpp"
#include "util/helpers.hpp"

namespace zc::app
{

class Update final : public AppCommand<"update", "Update packages">
{
  ZC_LIST_POSITIONAL(targets, std::string, "Packages to update");
  ZC_OPTION(local, stdfs::path, 'l', "Update from local project instead of remote", {});
  ZC_OPTION(jobs, u32, 'j', "Number of concurrent jobs for compilation", core::gc().jobs());
  ZC_FLAG(sync, 's', "Also add updated packages to project dependencies");
  ZC_FLAG(save, 'S', "Save path of local project in registry");
  ZC_FLAG(dont_use, 'd', "Do not set newly installed version as default version for package");

  void updateFromPath() const
  {
    if (local_->empty())
      return;

    core::Project project(*local_);
    auto          update_res = core::reg().updateFromPath(project, *jobs_, save_, force_, !dont_use_);
    if (!update_res)
      throw_or_terminate(update_res.error());
  }

  void updateFromServer(const std::map<std::string, Version, std::less<>> &targets) const
  {
    for (const auto &[pkg, ver] : targets) {
      auto update_res = core::reg().updateFromServer(pkg, ver, *jobs_, force_, !dont_use_);
      if (!update_res)
        throw_or_terminate(update_res.error());
    }
  }

public:
  Update() = default;

  [[nodiscard]] bool requiresProject() const override
  {
    return (targets_->empty() && local_->empty()) || sync_;
  }

  auto options() { return std::tie(targets_, local_, sync_, save_, dont_use_, jobs_); }

  void run() override
  {
    if (targets_->empty() && local_->empty()) {
      auto install_res = p().updateDependencies(*jobs_);
      if (!install_res)
        throw_or_terminate(install_res.error());
    } else {
      updateFromPath();
      auto parsed = parse_targets(*targets_);
      updateFromServer(parsed);
    }
  }
};

} // namespace zc::app
