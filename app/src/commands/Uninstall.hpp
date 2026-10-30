#pragma once

#include <filesystem>
#include <string>

#include "AppCommand.hpp"
#include "core/registry/Registry.hpp"
#include "framework/cli/Args.hpp"
#include "util/helpers.hpp"

namespace zc::app
{

class Uninstall final : public AppCommand<"uninstall", "Uninstall packages">
{
  ZC_LIST_POSITIONAL(targets, std::string, "Packages to uninstall");
  ZC_OPTION(local, stdfs::path, 'l', "Uninstall from local project instead of remote", {});
  ZC_FLAG(sync, 's', "Also remove uninstalled packages from project dependencies");

  void uninstallFromPath() const
  {
    if (local_->empty())
      return;

    const core::Project project(*local_);
    auto                install_res = core::reg().uninstallFromPath(project, force_);
    if (!install_res)
      throw_or_terminate(install_res.error());
  }

  void uninstallTargets(const std::map<std::string, Version, std::less<>> &targets) const
  {
    for (const auto &[pkg, ver] : targets) {
      if (ver.isEmpty()) {
        auto uninstall_res = core::reg().uninstall(pkg, force_);
        if (!uninstall_res)
          throw_or_terminate(uninstall_res.error());
      } else if (ver.isLatest()) {
        auto latest_res = core::reg().latestOf(pkg);
        if (!latest_res)
          throw_or_terminate(latest_res.error());
        if (latest_res->isEmpty())
          throw_or_terminate(
            Exception(core::ZCError::VersionNotFound, "Found no versions for package '{}'", pkg)
          );
        auto uninstall_res = core::reg().uninstall(pkg, *latest_res, force_);
        if (!uninstall_res)
          throw_or_terminate(uninstall_res.error());
      } else {
        auto uninstall_res = core::reg().uninstall(pkg, ver, force_);
        if (!uninstall_res)
          throw_or_terminate(uninstall_res.error());
      }
    }
  }

public:
  Uninstall() = default;

  [[nodiscard]] bool requiresProject() const override
  {
    return (targets_->empty() && local_->empty()) || sync_;
  }

  auto options() { return std::tie(targets_, local_, sync_); }

  void run() override
  {
    if (targets_->empty() && local_->empty()) {
      auto install_res = p().uninstallDependencies();
      if (!install_res)
        throw_or_terminate(install_res.error());
    } else {
      uninstallFromPath();
      auto parsed = parse_targets(*targets_);
      uninstallTargets(parsed);
    }
  }
};

} // namespace zc::app
