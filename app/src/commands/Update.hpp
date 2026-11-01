#pragma once

#include <filesystem>

#include "commands/PackageCommand.hpp"
#include "core/global-config/GlobalConfig.hpp"
#include "core/registry/Registry.hpp"
#include "framework/cli/Args.hpp"
#include "framework/Exception.hpp"
#include "framework/util/types.hpp"
#include "util/helpers.hpp"

namespace zc::app
{

class Update final : public PackageCommand<"update", "Update packages">
{
  ZC_OPTION(jobs, u32, 'j', "Number of concurrent jobs for compilation", core::gc().jobs());
  ZC_FLAG(save, 'S', "Save path of local project in registry");
  ZC_FLAG(dont_use, 'd', "Do not set newly installed version as default version for package");

  void updateFromPath() const
  {
    if (!local_)
      return;

    core::Project project(*local_);
    ZC_DO_OR_THROW(core::reg().updateFromPath(project, *jobs_, save_, force_, !dont_use_));
  }

  void updateFromServer()
  {
    const auto parsed = parse_targets(*packages_);
    for (const auto &[pkg, ver] : parsed)
      ZC_DO_OR_THROW(core::reg().updateFromServer(pkg, ver, *jobs_, force_, !dont_use_));
    // TODO: when not installed, install instead of crash
  }

public:
  Update() = default;

  auto options() { return std::tie(packages_, local_, sync_, save_, dont_use_, jobs_); }

  void run() override
  {
    if (actOnProject()) {
      ZC_DO_OR_THROW(p().updateDependencies(*jobs_));
    } else {
      updateFromPath();
      updateFromServer();
    }
  }
};

} // namespace zc::app
