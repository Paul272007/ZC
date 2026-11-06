#pragma once

#include <ranges>
#include <string_view>

#include "commands/AppCommand.hpp"
#include "core/registry/package_index.hpp"
#include "core/registry/Registry.hpp"
#include "framework/cli/Args.hpp"
#include "framework/ui/Table.hpp"
#include "util/helpers.hpp"
#include "util/ListType.hpp"

namespace zc::app
{

class List final : public AppCommand<"list", "List all sorts of things">
{
  ZC_POSITIONAL(type, std::string_view, "Type of things to list", "");
  ZC_FLAG(simple, 's', "Use simple list instead of table");

  static void displayPackagesTable()
  {
    if (core::reg().pkgs().empty()) {
      ui.println("No installed packages...");
      return;
    }

    std::vector<std::vector<std::string>> pkgs = {
      { "Name", "Default version", "Origin" },
    };
    for (const auto &[name, conf] : core::reg().pkgs())
      pkgs.push_back({ name, conf.default_version.string(), conf.origin });

    Table t(std::move(pkgs), false, true);
    t.setBorderThickness(Thickness::Rounded, Thickness::Rounded);
    t.draw();
  }

  static void displayPackages()
  {
    for (const auto &key : core::reg().pkgs() | std::views::keys)
      ui.println("{}", key);
  }

  static void displayRemotePackagesTable()
  {
    auto remote_pkgs_res = core::index::availablePkgs();
    if (!remote_pkgs_res)
      throw_or_terminate(remote_pkgs_res.error());

    if (remote_pkgs_res->empty()) {
      ui.println("No remote packages...");
      return;
    }

    std::vector<std::vector<std::string>> pkgs = {
      { "Name", "Latest version available" },
    };

    for (const auto &[k, v] : *remote_pkgs_res)
      pkgs.push_back({ k, v.string() });

    Table t(std::move(pkgs), false, true);
    t.setBorderThickness(Thickness::Rounded, Thickness::Rounded);
    t.draw();
  }

  static void displayRemotePackages()
  {
    auto remote_pkgs_res = core::index::availablePkgs();
    if (!remote_pkgs_res)
      throw_or_terminate(remote_pkgs_res.error());

    for (const auto &k : *remote_pkgs_res | std::views::keys)
      ui.println("{}", k);
  }

public:
  List() = default;

  [[nodiscard]] bool requiresProject() const override
  {
    return parse_list_type(*type_) == ListType::Dependencies;
  }

  auto options() { return std::tie(type_, simple_); }

  void run() override
  {
    const auto t = parse_list_type(*type_);
    if (t == ListType::Unknown)
      throw_or_terminate(Exception(CliError::InvalidValue, "unknown list type"));

    switch (t) {
    case ListType::ProjectTemplates:
    case ListType::FileTemplates:
      ui.debug("Not implemented");
      break;
    case ListType::Dependencies:
      // TODO: implement
      break;
    case ListType::Remote:
      if (simple_)
        displayRemotePackages();
      else
        displayRemotePackagesTable();
      break;
    case ListType::Packages:
    default:
      if (simple_)
        displayPackages();
      else
        displayPackagesTable();
      break;
    }
  }
};

} // namespace zc::app
