#pragma once

#include <filesystem>
#include <string>

#include "AppCommand.hpp"
#include "core/registry/Registry.hpp"
#include "framework/cli/Args.hpp"
#include "util/helpers.hpp"

namespace zc::app
{

class Use final : public AppCommand<"use", "Set default version for packages">
{
  ZC_LIST_POSITIONAL(targets, std::string, "Packages and their version to use");
  ZC_OPTION(local, stdfs::path, 'l', "Use version of this local project", {});
  ZC_FLAG(sync, 's', "For each project dependency, set its version as default");

  void useFromPath() const
  {
    if (local_->empty())
      return;

    const core::Project p(*local_);
    auto                use_res = core::reg().setDefaultVersion(p.name(), p.version());
    if (!use_res)
      throw_or_terminate(use_res.error());
  }

  static void useTargets(const std::map<std::string, Version, std::less<>> &targets)
  {
    for (const auto &[name, version] : targets) {
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
  Use() = default;

  [[nodiscard]] bool requiresProject() const override
  {
    return (targets_->empty() && local_->empty()) || sync_;
  }

  auto options() { return std::tie(targets_, local_, sync_); }

  void run() override
  {
    if (targets_->empty() && local_->empty()) {
      auto use_res = p().useDependencies();
      if (!use_res)
        throw_or_terminate(use_res.error());
    } else {
      useFromPath();
      auto parsed = parse_targets(*targets_);
      useTargets(parsed);
    }
  }
};

} // namespace zc::app
