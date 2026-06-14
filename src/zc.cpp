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

#include "CLI11.h"
#include "commands/Command.h"
#include "commands/Run.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "ui/Interface.h"

ZC_DEV_CONFIG

int main(const int argc, char *argv[])
{
  // --- Initialize app
  CLI::App app("ZC utility for C and C++");
  argv = app.ensure_utf8(argv);
  app.set_version_flag("--version,-v", "ZC v0.1.1 (unstable)");
  app.require_subcommand(1);
  unique_ptr<Command> command(nullptr);
  // clang-format off

  // --- Command line arguments as variables
  bool force  = false;
  bool quiet  = false;

  // bool edit   = false;
  // bool git    = false;
  // bool global = false;

  // --- Subcommands
  // Files
  const auto run     = app.add_subcommand("run",     "Compile and execute C/C++ file(s)");
  // const auto create  = app.add_subcommand("create",  "Create file based on template");
  // Projects
  // const auto init    = app.add_subcommand("init",    "Initialize empty project");
  // const auto build   = app.add_subcommand("build",   "Build project");
  // const auto add     = app.add_subcommand("add",     "Add dependencies from global registry");
  // const auto remove  = app.add_subcommand("remove",  "Remove packages");
  // const auto publish = app.add_subcommand("publish", "Publish package to server");
  // const auto clean   = app.add_subcommand("clean",   "Clean all temporary files and directories");
  // Global
  // const auto list    = app.add_subcommand("list",    "List installed libraries");
  // const auto install = app.add_subcommand("install", "Install packages");
  // const auto update  = app.add_subcommand("update",  "Update packages");
  // const auto login   = app.add_subcommand("login",   "Log into an account");
  // const auto logout  = app.add_subcommand("logout",  "Log out");

  // --- Subcommands arguments
  // Run

  run->add_flag("--quiet,-q", quiet, "Enable quiet mode");

  run->callback([&command] { command = make_unique<Run>(); });

  // clang-format on

  try
  {
    app.parse(argc, argv);
    Interface::get(quiet); // create Interface instance before any other command
    if (command)
      return (*command)();
  }
  catch (const CLI::ParseError &e)
  {
    return app.exit(e);
  }
  catch (const ZCException &e)
  {
    cerr << e << endl;
    return e.code();
  }
  catch (const exception &e)
  {
    cerr << RED << "Unexpected error: " << RESET << e.what() << endl;
    return -1;
  }
  return 0;
}
