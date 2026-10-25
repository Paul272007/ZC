#pragma once

#include <algorithm>

#include "BuildCommand.hpp"
#include "Command.hpp"
#include "core/project/target/ProjectBinTarget.hpp"
#include "core/util/ZCError.hpp"

namespace zc::app
{

class Build final : public Command, BuildCommand
{
  const bool clean_;
  const bool run_;

  const core::BuildMode    mode_;
  std::vector<std::string> args_;
  std::vector<std::string> targets_;

public:
  explicit Build(
    const CommandCtx &cmd_ctx, const BuildCommandCtx build_ctx, std::vector<std::string> targets,
    const bool clean, const bool run, const bool release, const bool debug, const bool ReleaseWithDebugInfo,
    std::vector<std::string> run_args
  )
    : Command(cmd_ctx, true),
      BuildCommand(build_ctx),
      clean_(clean),
      run_(run),
      mode_(parse_mode(
        {
          { core::BuildMode::Release, release },
          { core::BuildMode::Debug, debug },
          { core::BuildMode::ReleaseWithDebugInfo, ReleaseWithDebugInfo },
        },
        core::BuildMode::Auto, "cannot select multiple build modes"
      )),
      args_(std::move(run_args)),
      targets_(std::move(targets))
  {
  }

  void execute() override
  {
    if (clean_)
      p().clean();

    if (targets_.empty() || targets_.front() == "all") {
      // or deduce targets to build using stdfs::current_path() ?
      auto build_res = p().buildAll(mode_, jobs_);
      if (!build_res)
        throw_or_terminate(build_res.error());

      if (run_) {
        for (const auto &target : p().targets()) {
          if (const auto *bin = dynamic_cast<const core::ProjectBinTarget *>(target.get())) {
            const int code = bin->execute(args_);
            if (code != 0) {
              throw_or_terminate(
                Exception(core::ZCError::BuildFailed, "target '{}' exited with code {}", bin->name(), code)
              );
            }
          }
        }
      }
    } else {
      for (const auto &target : targets_) {
        auto build_res = p().buildTarget(target, mode_, jobs_);
        if (!build_res)
          throw_or_terminate(build_res.error());
      }

      if (run_) {
        for (const auto &target_name : targets_) {
          const auto it =
            std::ranges::find_if(p().targets(), [&](const auto &t) { return t->name() == target_name; });
          if (it == p().targets().end()) {
            throw_or_terminate(
              Exception(core::ZCError::TargetNotFound, "target '{}' not found in project", target_name)
            );
          }
          if (const auto *bin = dynamic_cast<const core::ProjectBinTarget *>(it->get())) {
            const int code = bin->execute(args_);
            if (code != 0) {
              throw_or_terminate(
                Exception(core::ZCError::BuildFailed, "target '{}' exited with code {}", bin->name(), code)
              );
            }
          } else {
            ui.warning("target '{}' is not an executable binary, skipping execution", target_name);
          }
        }
      }
    }
  }
};

} // namespace zc::app
