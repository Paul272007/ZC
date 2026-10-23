#pragma once

#include "BuildCommand.hpp"
#include "Command.hpp"
#include "core/registry/Registry.hpp"
#include "InstallCommand.hpp"

namespace zc::app
{

class Install final : public Command, InstallCommand, BuildCommand
{
  const bool save_path_;

  void installFromPath() const
  {
    if (target_path_.empty())
      return;

    core::Project project(target_path_);
    auto          install_res = core::reg().installFromPath(project, jobs_, save_path_, force_);
    if (!install_res)
      throw_or_terminate(install_res.error());

    // if (sync_)
    // p().add_dependency(LocalTarget::get_target({ project.pconf.name, project.pconf.version }));
  }

  void installFromServer() const
  {
    for (const auto &[pkg, ver] : targets_) {
      auto install_res = core::reg().installFromServer(pkg, ver, jobs_, force_);
      if (!install_res)
        throw_or_terminate(install_res.error());
      // TODO: sync
    }
  }

public:
  explicit Install(
    const CommandCtx &cmd_ctx, InstallCommandCtx &inst_ctx, const BuildCommandCtx build_ctx,
    const bool save_path
  )
    : Command(cmd_ctx, (inst_ctx.targets.empty() && inst_ctx.target_path.empty()) || inst_ctx.sync),
      InstallCommand(inst_ctx),
      BuildCommand(build_ctx),
      save_path_(save_path)
  {
  }

  void execute() override
  {
    // TODO: add installFromStd option
    // TODO: implement --sync flag
    if (targets_.empty() && target_path_.empty()) {
      auto install_res = p().installDependencies(jobs_);
      if (!install_res)
        throw_or_terminate(install_res.error());
    } else {
      installFromPath();
      installFromServer();
    }
  }
};

} // namespace zc::app
