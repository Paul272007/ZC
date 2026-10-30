#pragma once

#include <algorithm>
#include <string_view>

#include "commands/AppCommand.hpp"
#include "core/global-config/GlobalConfig.hpp"
#include "core/project/target/ProjectBinTarget.hpp"
#include "core/util/ZCError.hpp"
#include "framework/cli/Args.hpp"

namespace zc::app
{

class Build final : public AppCommand<"build", "Build project targets", true>
{
  ZC_LIST_POSITIONAL(targets, std::string_view, "Targets to build");

  ZC_FLAG(clean, 'c', "Clean build directory before building");
  ZC_FLAG(release, 'r', "Compile in Release mode");
  ZC_FLAG(debug, 'd', "Compile in Debug mode");
  ZC_FLAG(rel_with_deb_info, 'D', "Compile in Release with debug info mode");
  ZC_OPTION(jobs, u32, 'j', "Number of concurrent jobs for compilation", core::gc().jobs());
  ZC_LIST_OPTION(run, std::string_view, 'R', "Execute compiled binary targets with optional arguments");

  void runBin(const core::ProjectBinTarget &bin) const
  {
    const int code = bin.execute(*run_);
    if (code != 0)
      throw_or_terminate(
        Exception(core::ZCError::BuildFailed, "target '{}' exited with code {}", bin.name(), code)
      );
  }

public:
  Build() = default;

  auto options() { return std::tie(targets_, clean_, release_, debug_, rel_with_deb_info_, jobs_, run_); }

  void run() override
  {
    const auto mode = parse_mode(
      {
        { core::BuildMode::Release, *release_ },
        { core::BuildMode::Debug, *debug_ },
        { core::BuildMode::ReleaseWithDebugInfo, *rel_with_deb_info_ },
      },
      core::BuildMode::Auto, "cannot select multiple build modes"
    );

    if (clean_)
      p().clean();

    if (targets_->empty() || targets_->front() == "all") {
      // or deduce targets to build using stdfs::current_path() ?
      auto build_res = p().buildAll(mode, *jobs_);
      if (!build_res)
        throw_or_terminate(build_res.error());

      if (run_) {
        for (const auto &target : p().targets())
          if (const auto *bin = dynamic_cast<const core::ProjectBinTarget *>(target.get()))
            runBin(*bin);
      }
      return;
    }

    for (const auto &target : targets_) {
      auto build_res = p().buildTarget(target, mode, *jobs_);
      if (!build_res)
        throw_or_terminate(build_res.error());
    }

    if (!run_)
      return;

    for (const auto &name : targets_) {
      const auto it = std::ranges::find_if(p().targets(), [&](const auto &t) { return t->name() == name; });
      if (it == p().targets().end())
        throw_or_terminate(
          Exception(core::ZCError::TargetNotFound, "target '{}' not found in project", name)
        );

      if (const auto *bin = dynamic_cast<const core::ProjectBinTarget *>(it->get()))
        runBin(*bin);
      else
        ui.warning("target '{}' is not an executable binary, skipping execution", name);
    }
  }
};

} // namespace zc::app
