#pragma once

#include <filesystem>

#include "commands/PackageCommand.hpp"
#include "core/registry/Registry.hpp"
#include "framework/cli/Args.hpp"
#include "framework/Exception.hpp"
#include "util/helpers.hpp"

namespace zc::app
{

class Uninstall final : public PackageCommand<"uninstall", "Uninstall packages">
{
  void uninstallFromPath() const
  {
    if (!local_)
      return;

    const core::Project project(*local_);
    ZC_DO_OR_THROW(core::reg().uninstallFromPath(project, force_));
  }

  void uninstallTargets()
  {
    const auto parsed = parse_targets(*packages_);
    for (const auto &[pkg, ver] : parsed) {
      if (ver.isEmpty()) {
        ZC_DO_OR_THROW(core::reg().uninstall(pkg, force_));
      } else if (ver.isLatest()) {
        auto latest_res = core::reg().latestOf(pkg);
        if (!latest_res)
          throw_or_terminate(latest_res.error());

        if (latest_res->isEmpty())
          throw_or_terminate(
            Exception(core::ZCError::VersionNotFound, "Found no versions for package '{}'", pkg)
          );
        ZC_DO_OR_THROW(core::reg().uninstall(pkg, *latest_res, force_));
      } else {
        ZC_DO_OR_THROW(core::reg().uninstall(pkg, ver, force_));
      }
    }
  }

public:
  Uninstall() = default;

  auto options() { return std::tie(packages_, local_, sync_); }

  void run() override
  {
    if (actOnProject()) {
      ZC_DO_OR_THROW(p().uninstallDependencies());
    } else {
      uninstallFromPath();
      uninstallTargets();
    }
  }
};

} // namespace zc::app
