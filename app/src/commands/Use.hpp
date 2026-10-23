#pragma once

#include "Command.hpp"
#include "core/registry/Registry.hpp"
#include "InstallCommand.hpp"

namespace zc::app
{

class Use final : public Command, InstallCommand
{
  void useFromPath() const
  {
    if (target_path_.empty())
      return;

    const core::Project p(target_path_);
    auto                use_res = core::reg().setDefaultVersion(p.name(), p.version());
    if (!use_res)
      throw_or_terminate(use_res.error());
  }

  void useTargets() const
  {
    for (const auto &[name, version] : targets_) {
      Version v;
      if (version.isEmpty() || version.isLatest()) {
        auto latest_res = core::reg().latestOf(name);
        if (!latest_res)
          throw_or_terminate(latest_res.error());
        v = *latest_res;
      } else {
        v = version;
      }
      auto use_res = core::reg().setDefaultVersion(name, v);
      if (!use_res)
        throw_or_terminate(use_res.error());
    }
  }

public:
  explicit Use(const CommandCtx &cmd_ctx, InstallCommandCtx &inst_ctx)
    : Command(cmd_ctx, (inst_ctx.targets.empty() && inst_ctx.target_path.empty()) || inst_ctx.sync),
      InstallCommand(inst_ctx)
  {
  }

  void execute() override
  {
    if (targets_.empty() && target_path_.empty()) {
      auto use_res = p().useDependencies();
      if (!use_res)
        throw_or_terminate(use_res.error());
    } else {
      useFromPath();
      useTargets();
    }
  }
};

} // namespace zc::app
