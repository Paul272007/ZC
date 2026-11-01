#pragma once

#include <filesystem>

#include "commands/PackageCommand.hpp"
#include "core/global-config/GlobalConfig.hpp"
#include "core/registry/Registry.hpp"
#include "framework/cli/Args.hpp"
#include "framework/Exception.hpp"
#include "util/helpers.hpp"

namespace zc::app
{

class Install final : public PackageCommand<"install", "Install packages">
{
  ZC_FLAG(save, 'S', "Save path of local project in registry");
  ZC_OPTION(jobs, u32, 'j', "Number of concurrent jobs for compilation", core::gc().jobs());

  void installFromPath() const
  {
    if (!local_)
      return;

    core::Project project(*local_);
    ZC_DO_OR_THROW(core::reg().installFromPath(project, *jobs_, save_, force_));
  }

  void installFromServer()
  {
    const auto parsed = parse_targets(*packages_);
    for (const auto &[pkg, ver] : parsed)
      ZC_DO_OR_THROW(core::reg().installFromServer(pkg, ver, *jobs_, force_));
  }

public:
  Install() = default;

  auto options() { return std::tie(packages_, local_, sync_, save_, jobs_); }

  void run() override
  {
    if (actOnProject()) {
      ZC_DO_OR_THROW(p().installDependencies(*jobs_));
    } else {
      installFromPath();
      installFromServer();
    }
  }
};

} // namespace zc::app
