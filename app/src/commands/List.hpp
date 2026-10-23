#pragma once

#include "commands/Command.hpp"
#include "core/registry/package_index.hpp"
#include "core/registry/Registry.hpp"
#include "framework/ui/Table.hpp"
#include "util/helpers.hpp"
#include "util/ListType.hpp"

namespace zc::app
{

class List final : public Command
{
  ListType type_;
  bool     simple_;

  explicit List(const CommandCtx &ctx, const ListType &type, const bool simple)
    : Command(ctx, type == ListType::Dependencies), type_(type), simple_(simple)
  {
    if (type_ == ListType::Unknown)
      throw_or_terminate(Exception(CliError::InvalidValue, "unknown list type"));
  }

  static void displayPackagesTable()
  {
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
    std::vector<std::vector<std::string>> pkgs = {
      { "Name", "Latest version available" },
    };

    auto remote_pkgs_res = core::index::availablePkgs();
    if (!remote_pkgs_res)
      throw_or_terminate(remote_pkgs_res.error());

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
  explicit List(const CommandCtx &ctx, const std::string_view type, const bool simple)
    : List(ctx, parse_type(type), simple)
  {
  }

  void execute() override
  {
    switch (type_) {
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
