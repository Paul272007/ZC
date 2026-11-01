#pragma once

#include <cstddef>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "commands/AppCommand.hpp"
#include "core/global-config/GlobalConfig.hpp"
#include "core/project/Link.hpp"
#include "core/project/target/ProjectTarget.hpp"
#include "core/project/target/ProjectTargetDependency.hpp"
#include "core/registry/package_index.hpp"
#include "core/registry/Registry.hpp"
#include "core/registry/RegistryPkg.hpp"
#include "core/registry/RegistryProject.hpp"
#include "core/util/ZCError.hpp"
#include "framework/cli/Args.hpp"
#include "framework/Exception.hpp"
#include "framework/ui/UI.hpp"
#include "framework/util/errors.hpp"
#include "framework/util/types.hpp"
#include "util/helpers.hpp"

namespace zc::app
{

class Add final : public AppCommand<"add", "Add dependencies to project target", true>
{
  ZC_LIST_POSITIONAL(packages, std::string_view, "Dependencies to add");
  ZC_OPTION(target, std::string_view, 't', "Project target to add dependencies to", {});
  ZC_OPTION(jobs, u32, 'j', "Number of concurrent jobs for compilation", core::gc().jobs());
  ZC_FLAG(static, 's', "Add static version of libraries");

  [[nodiscard]] core::ProjectTarget *getDestTarget() const
  {
    if (target_ && !target_->empty()) {
      auto *found = p().findTarget(*target_);
      if (!found)
        throw_or_terminate(
          Exception(core::ZCError::TargetNotFound, "Target '{}' not found in project", *target_)
        );
      return found;
    }

    if (p().targets().empty())
      throw_or_terminate(Exception(core::ZCError::TargetNotFound, "No targets found in project"));

    if (p().targets().size() == 1) {
      ui.info("Only one candidate target found: {}", p().targets().front()->name());
      return p().targets().front().get();
    }

    if (auto *detected = detectedTarget())
      return detected;

    if (quiet_)
      throw_or_terminate(
        Exception(CliError::MissingFlag, "ambiguous target: specify one with --target/-t")
      );

    std::vector<std::string_view> options;
    options.reserve(p().targets().size());
    for (const auto &t : p().targets())
      options.emplace_back(t->name());

    const u64 selected = ui.radios(options, 0, "Target to add dependencies to:");
    return p().targets().at(selected).get();
  }

  void confirmInstall(std::string_view pkg_name, Version ver) const
  {
    if (force_)
      return;

    if (quiet_) {
      if (ver.isEmpty())
        throw_or_terminate(Exception(
          core::ZCError::PkgNotFound, "Package '{}' is not installed (use -f to install)", pkg_name
        ));
      throw_or_terminate(Exception(
        core::ZCError::PkgNotFound, "Package '{}@{}' is not installed (use -f to install)", pkg_name, ver
      ));
    }

    const bool accepted = ver.isEmpty()
      ? ui.ask(true, "Package '{}' is not installed. Install it?", pkg_name)
      : ui.ask(true, "Package '{}@{}' is not installed. Install it?", pkg_name, ver);
    if (!accepted)
      throw_or_terminate(Exception(CliError::Interrupted, "Aborted"));
  }

  [[nodiscard]] std::pair<const core::RegistryPkg *, Version> ensureInstalled(const DepSpec &spec) const
  {
    if (spec.version.isEmpty()) {
      if (core::reg().isInstalled(spec.package)) {
        const auto *pkg = core::reg().getPkg(spec.package);
        return { pkg, pkg->default_version };
      }

      confirmInstall(spec.package, Version::empty());
      auto res = core::reg().installFromServer(spec.package, Version::latest(), *jobs_, force_);
      if (!res)
        throw_or_terminate(res.error());
      return { *res, (*res)->default_version };
    }

    Version desired_version = spec.version;
    if (spec.version.isLatest()) {
      const auto *existing_pkg = core::reg().getPkg(spec.package);
      if (existing_pkg && existing_pkg->origin != "main") {
        auto latest_res = core::reg().latestOf(spec.package);
        if (!latest_res)
          throw_or_terminate(latest_res.error());
        desired_version = *latest_res;
      } else {
        auto remote_res = core::index::resolve(spec.package, Version::latest());
        if (!remote_res)
          throw_or_terminate(remote_res.error());
        desired_version = remote_res->version;
      }
    }

    if (core::reg().isInstalled(spec.package, desired_version))
      return { core::reg().getPkg(spec.package), desired_version };

    confirmInstall(spec.package, desired_version);

    auto res = core::reg().isInstalled(spec.package)
      ? core::reg().updateFromServer(spec.package, desired_version, *jobs_, force_, false)
      : core::reg().installFromServer(spec.package, desired_version, *jobs_, force_);
    if (!res)
      throw_or_terminate(res.error());

    return { *res, desired_version };
  }

  void chooseTargets(
    core::ProjectTargetDependency &dep, const core::RegistryProject &snapshot, core::Link link
  ) const
  {
    std::vector<std::string_view> options;
    for (const auto &t : snapshot.targets)
      if (t->canBeADependency())
        options.emplace_back(t->name);

    if (options.empty())
      throw_or_terminate(Exception(
        core::ZCError::TargetNotFound, "Package '{}@{}' exposes no dependency targets", dep.package,
        dep.version
      ));

    if (options.size() == 1) {
      ui.info("Added only available target of package '{}': '{}'", dep.package, options.front());
      dep.targets.emplace_back(options.front(), link);
      return;
    }

    if (quiet_)
      throw_or_terminate(Exception(
        CliError::MissingValue,
        "Package '{}@{}' has multiple targets: specify one with '{}/<target>' or '{}/*'", dep.package,
        dep.version, dep.package, dep.package
      ));

    std::vector<bool> selected(options.size(), false);
    ui.checkboxes(options, selected, "Choose targets of '{}@{}' to add:", dep.package, dep.version);

    for (std::size_t i = 0; i < options.size(); ++i)
      if (selected[i])
        dep.targets.emplace_back(options[i], link);
  }

public:
  Add() = default;

  [[nodiscard]] auto options() { return std::tie(packages_, target_, jobs_, static_); }

  void run() override
  {
    if (!packages_ || packages_->empty())
      throw_or_terminate(Exception(CliError::MissingPositional, "expected at least one dependency to add"));

    core::ProjectTarget *dest_target = getDestTarget();
    const core::Link     link        = static_ ? core::Link::Static : core::Link::Dynamic;

    for (const auto &raw : *packages_) {
      const DepSpec spec = parse_dep_spec(raw);

      const auto [pkg, resolved_ver] = ensureInstalled(spec);
      if (!pkg)
        throw_or_terminate(
          Exception(core::ZCError::PkgNotFound, "Package '{}' was not found", spec.package)
        );

      const core::RegistryProject *snapshot = pkg->getVersion(resolved_ver);
      if (!snapshot)
        throw_or_terminate(
          Exception(core::ZCError::PkgNotFound, "Package '{}@{}' was not found", pkg->name, resolved_ver)
        );

      core::ProjectTargetDependency dep{
        .package = pkg->name,
        .version = resolved_ver,
        .targets = {},
      };

      if (spec.target.empty()) {
        chooseTargets(dep, *snapshot, link);
        if (dep.targets.empty()) {
          ui.warning("No target selected for '{}@{}', skipping.", dep.package, dep.version);
          continue;
        }
      } else if (spec.target == "*") {
        for (const auto &t : snapshot->targets)
          if (t->canBeADependency())
            dep.targets.emplace_back(t->name, link);

        if (dep.targets.empty())
          throw_or_terminate(Exception(
            core::ZCError::TargetNotFound, "Package '{}@{}' exposes no dependency targets", dep.package,
            dep.version
          ));
      } else {
        const auto *reg_target = snapshot->findTarget(spec.target);
        if (!reg_target)
          throw_or_terminate(Exception(
            core::ZCError::TargetNotFound, "Target '{}' of package '{}@{}' was not found", spec.target,
            dep.package, dep.version
          ));
        if (!reg_target->canBeADependency())
          throw_or_terminate(Exception(
            core::ZCError::InvalidValue, "Target '{}' of package '{}@{}' cannot be added as a dependency",
            spec.target, dep.package, dep.version
          ));
        dep.targets.emplace_back(reg_target->name, link);
      }

      dest_target->addDependency(std::move(dep));
    }

    dest_target->save();
  }
};

} // namespace zc::app
