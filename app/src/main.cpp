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

#include "commands/Build.hpp"
#include "commands/Command.hpp"
#include "commands/Config.hpp"
#include "commands/Init.hpp"
#include "commands/Install.hpp"
#include "commands/List.hpp"
#include "commands/Login.hpp"
#include "commands/Logout.hpp"
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

  bool save_path               = false;             // install / update
  bool dont_use                = false;             // update
  bool simple                  = false;             // list
  bool git                     = false;             // init
  bool edit                    = false;             // init
  bool clean                   = false;             // build
  bool release                 = false;             // build
  bool debug                   = false;             // build
  bool release_with_debug_info = false;             // build

  std::string_view type;                            // list
  std::string_view key;                             // config
  std::string_view value;                           // config
  std::string_view name;                            // init
  std::string_view author;                          // init
  std::string_view description;                     // init
  std::string_view project_template;                // init

  std::vector<std::string_view> languages;          // init
  std::vector<std::string>      build_targets;      // build

  std::optional<std::vector<std::string>> run_args; // build

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

  const auto config =
    zc::cli::cmd<"config", "Manage global configuration">()
      .requiredPositional<"key">(key, "Configuration key to modify")
      .positional<"value">(value, "Value given to this key")
      .setAction([&] { command = std::make_unique<zc::app::Config>(cmd_ctx, key, value); });

  const auto login = zc::cli::cmd<"login", "Log into your Github account">().setAction([&] {
    command = std::make_unique<zc::app::Login>(cmd_ctx);
  });

  const auto logout = zc::cli::cmd<"logout", "Log out of your account">().setAction([&] {
    command = std::make_unique<zc::app::Logout>(cmd_ctx);
  });

  // ——— Commands on projects ———

  const auto init = zc::cli::cmd<"init", "Create and initialize new project">()
                      .flag<"git", 'g'>(git, "Also initialize git repository")
                      .flag<"edit", 'E'>(edit, "Open project in editor once initialized")
                      .flag<"name", 'n'>(name, "New project")
                      .flag<"author", 'a'>(author, "Set project author")
                      .flag<"description", 'd'>(description, "Set project description")
                      .flag<"template", 't'>(project_template, "Project template to use")
                      .flag<"languages", 'l'>(languages, "Set project languages")
                      .setAction([&] {
                        command = std::make_unique<zc::app::Init>(
                          cmd_ctx, git, edit, name, author, description, project_template, languages
                        );
                      });

  const auto build =
    zc::cli::cmd<"build", "Build project targets">()
      .positional<"targets">(build_targets, "Targets to build")
      .flag<"clean", 'c'>(clean, "Clean build directory before building")
      .flag<"release", 'r'>(release, "Compile in Release mode")
      .flag<"debug", 'd'>(debug, "Compile in Debug mode")
      .flag<"release-with-debug-info", 'D'>(
        release_with_debug_info, "Compile in Release with debug info mode"
      )
      .flagWithDefault<"jobs", 'j'>(
        build_ctx.jobs, zc::core::gc().jobs(), "Number of concurrent jobs for compilation"
      )
      .flag<"run", 'R'>(run_args, "Execute compiled binary targets with optional arguments")
      .setAction([&] {
        const bool               run  = run_args.has_value();
        std::vector<std::string> args = run_args.value_or(std::vector<std::string>{});
        command                       = std::make_unique<zc::app::Build>(
          cmd_ctx, build_ctx, std::move(build_targets), clean, run, release, debug, release_with_debug_info,
          std::move(args)
        );
      });

  // ——— Root command ———

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
                          .subcommand(config)
                          .subcommand(login)
                          .subcommand(logout)
                          .subcommand(init)
                          .subcommand(build)
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
