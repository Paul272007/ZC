// clang-format off
// ZC: Version 0.1.1
//
//   ________  ________
//  |\_____  \|\   ____\
//   \|___/  /\ \  \___|
//       /  / /\ \  \
//      /  /_/__\ \  \____
//     |\________\ \_______\
//      \|_______|\|_______|
//
// Copyright (c) 2026 Paul Maillard. All Rights Reserved.
// clang-format on

#include <exception>

#include "commands/Command.hpp"
#include "commands/Install.hpp"
#include "commands/List.hpp"
#include "commands/Uninstall.hpp"
#include "commands/Update.hpp"
#include "commands/Use.hpp"
#include "context/BuildCommandCtx.hpp"
#include "context/InstallCommandCtx.hpp"
#include "core/global-config/GlobalConfig.hpp"
#include "framework/cli/AppBuilder.hpp"
#include "framework/cli/CommandBuilder.hpp"
#include "framework/Exception.hpp"
#include "framework/Timer.hpp"
#include "framework/ui/UI.hpp"
#include "framework/util/terminal.hpp"

int main(const int argc, char **argv)
{
  ZC_DEBUG_TIMER("Total time")

#ifdef ZC_DEBUG
  constexpr std::string_view version_str = "ZC v" ZC_VERSION " (debug)";
#else
  constexpr std::string_view version_str = "ZC v" ZC_VERSION;
#endif

  zc::app::CommandCtx        cmd_ctx;
  zc::app::InstallCommandCtx install_ctx;
  zc::app::BuildCommandCtx   build_ctx;

  bool save_path = false; // install / update
  bool dont_use  = false; // update
  bool simple    = false; // list

  std::string_view type;  // list

  std::unique_ptr<zc::app::Command> command;

  // ——— Commands on packages ———

  const auto install =
    zc::cli::cmd<"install", "Install packages">()
      .positional<"targets">(install_ctx.targets, "Packages to install")
      .flag<"local", 'l'>(install_ctx.target_path, "Install from local project instead of remote")
      .flag<"sync", 's'>(install_ctx.sync, "Also add installed packages to project dependencies")
      .flag<"save", 'S'>(save_path, "Save path of local project in registry")
      .flagWithDefault<"jobs", 'j'>(
        build_ctx.jobs, zc::core::gc().jobs(), "Number of concurrent jobs for compilation"
      )
      .setAction([&] {
        command = std::make_unique<zc::app::Install>(cmd_ctx, install_ctx, build_ctx, save_path);
      });

  const auto update =
    zc::cli::cmd<"update", "Update packages">()
      .positional<"targets">(install_ctx.targets, "Packages to update")
      .flag<"local", 'l'>(install_ctx.target_path, "Update from local project instead of remote")
      .flag<"sync", 's'>(install_ctx.sync, "Also add updated packages to project dependencies")
      .flag<"save", 'S'>(save_path, "Save path of local project in registry")
      .flag<"dont-use", 'd'>(dont_use, "Do not set newly installed version as default version for package")
      .flagWithDefault<"jobs", 'j'>(
        build_ctx.jobs, zc::core::gc().jobs(), "Number of concurrent jobs for compilation"
      )
      .setAction([&] {
        command = std::make_unique<zc::app::Update>(cmd_ctx, install_ctx, build_ctx, save_path, !dont_use);
      });

  const auto uninstall =
    zc::cli::cmd<"uninstall", "Uninstall packages">()
      .positional<"targets">(install_ctx.targets, "Packages to uninstall")
      .flag<"local", 'l'>(install_ctx.target_path, "Uninstall from local project instead of remote")
      .flag<"sync", 's'>(install_ctx.sync, "Also remove uninstalled packages from project dependencies")
      .setAction([&] { command = std::make_unique<zc::app::Uninstall>(cmd_ctx, install_ctx); });

  const auto use =
    zc::cli::cmd<"use", "Set default version for packages">()
      .positional<"targets">(install_ctx.targets, "Packages and their version to use")
      .flag<"local", 'l'>(install_ctx.target_path, "Use version of this local project")
      .flag<"sync", 's'>(install_ctx.sync, "For each project dependency, set its version as default")
      .setAction([&] { command = std::make_unique<zc::app::Use>(cmd_ctx, install_ctx); });

  // ——— Commands on global config ———

  const auto list = zc::cli::cmd<"list", "List all sorts of things">()
                      .positional<"type">(type, "Type of things to list")
                      .flag<"simple", 's'>(simple, "Use simple list instead of table")
                      .setAction([&] { command = std::make_unique<zc::app::List>(cmd_ctx, type, simple); });

  const auto root_cmd = zc::cli::cmd<"zc", "The C and C++ package manager and build system!">()
                          .global<"quiet", 'q'>(cmd_ctx.quiet, "Enable quiet mode")
                          .global<"verbose", 'v'>(cmd_ctx.verbose, "Enable verbosity")
                          .global<"force", 'f'>(cmd_ctx.force, "Force execution and skip usual warnings")
                          .global<"input", 'i'>(cmd_ctx.in_path, "Input file")
                          .global<"output", 'o'>(cmd_ctx.out_path, "Output file")
                          .global<"error", 'e'>(cmd_ctx.err_path, "Error file")
                          .global<"project-path", 'p'>(cmd_ctx.project_path, "Path to project")
                          .subcommand(install)
                          .subcommand(update)
                          .subcommand(uninstall)
                          .subcommand(use)
                          .subcommand(list)
                          .requiresSubcommand();

  const auto app = zc::cli::app(root_cmd).version(version_str);

  try {
    app.parse(argc, argv);
    if (command)
      command->execute();
    return 0;
  } catch (const zc::Exception &e) {
    zc::ui.printException(e);
    return e.code();
  } catch (const std::exception &e) {
    zc::ui.error("{}Unexpected fatal error:{} {}", zc::red, zc::reset, e.what());
    return 255;
  }
}
