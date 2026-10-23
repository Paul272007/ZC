#pragma once

#include "commands/Command.hpp"
#include "commands/InstallCommand.hpp"
#include "core/registry/Registry.hpp"

namespace zc::app
{

class Uninstall final : public Command, InstallCommand
{
  void uninstallFromPath() const
  {
    if (target_path_.empty())
      return;

    const core::Project project(target_path_);
    auto                install_res = core::reg().uninstallFromPath(project, force_);
    if (!install_res)
      throw_or_terminate(install_res.error());
  }

  void uninstallTargets() const
  {
    for (const auto &[pkg, ver] : targets_) {
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
  explicit Uninstall(const CommandCtx &cmd_ctx, InstallCommandCtx &inst_ctx)
    : Command(cmd_ctx, (inst_ctx.targets.empty() && inst_ctx.target_path.empty()) || inst_ctx.sync),
      InstallCommand(inst_ctx)
  {
  }

  void execute() override
  {
    // TODO: implement --sync flag
    if (targets_.empty() && target_path_.empty()) {
      auto install_res = p().uninstallDependencies();
      if (!install_res)
        throw_or_terminate(install_res.error());
    } else {
      uninstallFromPath();
      uninstallTargets();
    }
  }
};

} // namespace zc::app
