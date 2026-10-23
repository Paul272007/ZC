#pragma once

#include "commands/BuildCommand.hpp"
#include "commands/Command.hpp"
#include "commands/InstallCommand.hpp"
#include "core/registry/Registry.hpp"

namespace zc::app
{

class Update final : public Command, InstallCommand, BuildCommand
{
  const bool save_path_;
  const bool set_as_default_;

  void updateFromPath() const
  {
    if (target_path_.empty())
      return;

    core::Project project(target_path_);
    auto update_res = core::reg().updateFromPath(project, jobs_, save_path_, force_, set_as_default_);
    if (!update_res)
      throw_or_terminate(update_res.error());
  }

  void updateFromServer() const
  {
    for (const auto &[pkg, ver] : targets_) {
      auto update_res = core::reg().updateFromServer(pkg, ver, jobs_, force_, set_as_default_);
      if (!update_res)
        throw_or_terminate(update_res.error());
      // TODO: sync
    }
  }

public:
  explicit Update(
    const CommandCtx &cmd_ctx, InstallCommandCtx &inst_ctx, const BuildCommandCtx build_ctx,
    const bool save_path, const bool set_as_default
  )
    : Command(cmd_ctx, (inst_ctx.targets.empty() && inst_ctx.target_path.empty()) || inst_ctx.sync),
      InstallCommand(inst_ctx),
      BuildCommand(build_ctx),
      save_path_(save_path),
      set_as_default_(set_as_default)
  {
  }

  void execute() override
  {
    // TODO: implement --sync flag
    if (targets_.empty() && target_path_.empty()) {
      auto install_res = p().updateDependencies(jobs_);
      if (!install_res)
        throw_or_terminate(install_res.error());
    } else {
      updateFromPath();
      updateFromServer();
    }
  }
};

} // namespace zc::app
