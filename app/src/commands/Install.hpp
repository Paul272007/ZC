#pragma once

#include <filesystem>
#include <string_view>

#include "commands/AppCommand.hpp"
#include "core/global-config/GlobalConfig.hpp"
#include "core/registry/Registry.hpp"
#include "framework/cli/Args.hpp"
#include "util/helpers.hpp"

namespace zc::app
{

class Install final : public AppCommand<"install", "Install packages">
{
  ZC_LIST_POSITIONAL(targets, std::string, "Packages to install");

  ZC_FLAG(sync, 's', "Also add installed packages to project dependencies");
  ZC_FLAG(save, 'S', "Save path of local project in registry");
  ZC_OPTION(local, stdfs::path, 'l', "Install from local project instead of remote", {});
  ZC_OPTION(jobs, u32, 'j', "Number of concurrent jobs for compilation", core::gc().jobs());

  void installFromPath() const
  {
    if (local_->empty())
      return;

    core::Project project(*local_);
    auto          install_res = core::reg().installFromPath(project, *jobs_, save_, force_);
    if (!install_res)
      throw_or_terminate(install_res.error());
  }

  void installFromServer(const std::map<std::string, Version, std::less<>> &targets) const
  {
    for (const auto &[pkg, ver] : targets) {
      auto install_res = core::reg().installFromServer(pkg, ver, *jobs_, force_);
      if (!install_res)
        throw_or_terminate(install_res.error());
    }
  }

public:
  Install() = default;

  [[nodiscard]] bool requiresProject() const override
  {
    return (targets_->empty() && local_->empty()) || sync_;
  }

  auto options() { return std::tie(targets_, local_, sync_, save_, jobs_); }

  void run() override
  {
    if (targets_->empty() && local_->empty()) {
      auto install_res = p().installDependencies(*jobs_);
      if (!install_res)
        throw_or_terminate(install_res.error());
    } else {
      installFromPath();
      auto parsed = parse_targets(*targets_);
      installFromServer(parsed);
    }
  }
};

} // namespace zc::app
