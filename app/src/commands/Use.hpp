#pragma once

#include <filesystem>

#include "core/registry/Registry.hpp"
#include "framework/cli/Args.hpp"
#include "framework/Exception.hpp"
#include "PackageCommand.hpp"
#include "util/helpers.hpp"

namespace zc::app
{

class Use final : public PackageCommand<"use", "Set default version for packages">
{
  void useFromPath() const
  {
    if (!local_)
      return;

    const core::Project p(*local_);
    ZC_DO_OR_THROW(core::reg().setDefaultVersion(p.name(), p.version()));
  }

  void useTargets()
  {
    const auto parsed = parse_targets(*packages_);
    for (const auto &[name, version] : parsed) {
      Version v;
      if (version.isEmpty() || version.isLatest()) {
        auto latest_res = core::reg().latestOf(name);
        if (!latest_res)
          throw_or_terminate(latest_res.error());
        v = *latest_res;
      } else {
        v = version;
      }
      ZC_DO_OR_THROW(core::reg().setDefaultVersion(name, v));
    }
  }

public:
  Use() = default;

  auto options() { return std::tie(packages_, local_, sync_); }

  void run() override
  {
    if (actOnProject()) {
      ZC_DO_OR_THROW(p().useDependencies());
    } else {
      useFromPath();
      useTargets();
    }
  }
};

} // namespace zc::app
