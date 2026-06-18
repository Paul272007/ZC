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
#include "commands/Add.h"
#include "commands/Build.h"
#include "commands/Clean.h"
#include "commands/Command.h"
#include "commands/Init.h"
#include "commands/Install.h"
#include "commands/List.h"
#include "commands/Login.h"
#include "commands/Logout.h"
#include "commands/Publish.h"
#include "commands/Remove.h"
#include "commands/Run.h"
#include "commands/Uninstall.h"
#include "commands/Update.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "ui/Interface.h"
#include "ui/ui_utils.h"

ZC_DEV_CONFIG

using namespace zc;

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
  bool quiet            = false;
  bool force            = false;
  // Build
  bool clean_before     = false;
  bool release          = false;
  // Init
  bool git              = false;
  bool edit             = false;
  bool is_bin           = false;
  bool is_lib           = false;
  bool is_header        = false;
  bool is_compose       = false;
  // List
  bool show_templates   = false;
  bool show_p_templates = false;
  bool simple_display   = false;
  bool show_remote      = false;

  string author;
  string target;
  string p_template;
  string name;
  string path;
  string p_root     = "."; // Default path to search for project root is current path

  vector<string> targets;

  // --- Subcommands
  // Files
  const auto run       = app.add_subcommand("run",       "Compile and execute C/C++ file(s)"); // TODO : implement
  // const auto create  = app.add_subcommand("create",  "Create file based on template"²);
  // Projects
  const auto init      = app.add_subcommand("init",      "Initialize empty project");
  const auto build     = app.add_subcommand("build",     "Build project");
  const auto add       = app.add_subcommand("add",       "Add dependencies to project");
  const auto remove    = app.add_subcommand("remove",    "Remove dependencies from project");
  const auto publish   = app.add_subcommand("publish",   "Publish package to server");
  const auto clean     = app.add_subcommand("clean",     "Clean all temporary files and directories");
  // Packages
  const auto list      = app.add_subcommand("list",      "List installed libraries");
  const auto install   = app.add_subcommand("install",   "Install packages");
  const auto uninstall = app.add_subcommand("uninstall", "Uninstall packages");
  const auto update    = app.add_subcommand("update",    "Update packages");
  // Configuration
  const auto login     = app.add_subcommand("login",     "Log into an account");
  const auto logout    = app.add_subcommand("logout",    "Log out");

  // --- Subcommands arguments
  // Run
  // TODO : add --force,-f flag for each command

  run->add_flag("--quiet,-q", quiet, "Enable quiet mode");

  run->callback([&] { command = make_unique<Run>(force); });

  // Init

  init->add_option("--project-path,-P", p_root, "Directory to use as project root");
  init->add_option("--author,-a", author, "Package author");
  init->add_option("--target,-t", target, "Package target");
  init->add_option("--project-template,-p", p_template, "Project template to use");
  init->add_option("--name,-n", name, "Name of the package");

  init->add_flag("--git,-g", git, "Initialize empty git repository at project root");
  init->add_flag("--edit,-e", edit, "Open project in editor once initialized");
  init->add_flag("--bin,-B", is_bin, "Make package of type BIN");
  init->add_flag("--lib,-L", is_lib, "Make package of type LIB");
  init->add_flag("--header,-H", is_header, "Make package of type HEADER");
  init->add_flag("--is_compose,-C", is_compose, "Make package of type COMPOSE");

  init->callback([&] { command = make_unique<Init>(force, p_root, git, edit, author, target, p_template, name, is_bin, is_lib, is_header, is_compose); });

  // Build

  build->add_option("--project-path,-P", p_root, "Directory to use as project root");

  build->add_flag("--quiet,-q", quiet, "Do not show any messages");
  build->add_flag("--clean,-c", clean_before, "Clean before building");
  build->add_flag("--release,-r", release, "Build in release mode instead of debug mode");

  build->callback([&] { command = make_unique<Build>(force, p_root, clean_before, release); });

  // Add

  add->add_option("--project-path,-P", p_root, "Directory to use as project root");
  add->add_option("targets", targets, "The dependencies to be added")->required();

  add->add_flag("--quiet,-q", quiet, "Do not show any messages");

  add->callback([&] { command = make_unique<Add>(force, p_root, targets); });

  // Remove

  remove->add_option("--project-path,-P", p_root, "Directory to use as project root");
  remove->add_option("targets", targets, "The dependencies to be removed")->required();

  remove->add_flag("--quiet,-q", quiet, "Do not show any messages");

  remove->callback([&] { command = make_unique<Remove>(force, p_root, targets); });

  // Publish

  publish->add_option("--project-path,-P", p_root, "Directory to use as project root");

  publish->add_flag("--quiet,-q", quiet, "Do not show any messages");

  publish->callback([&] { command = make_unique<Publish>(force, p_root); });

  // Clean

  clean->add_option("--project-path,-P", p_root, "Directory to use as project root");

  clean->add_flag("--quiet,-q", quiet, "Do not show any messages");

  clean->callback([&] { command = make_unique<Clean>(force, p_root); });

  // List

  list->add_flag("--templates,-t", show_templates, "Show available templates instead of packages");
  list->add_flag("--project-templates,-p", show_p_templates, "Show available project templates instead of packages");
  list->add_flag("--remote,-r", show_remote, "Show remote packages instead of local ones");
  list->add_flag("--simple,-s", simple_display, "Use a simpler display");

  list->callback([&] { command = make_unique<List>(force, show_templates, show_p_templates, show_remote, simple_display); });

  // Install
  // TODO : add --project-path

  install->add_option("--path,-p", path, "Install from local project instead of remote");
  install->add_option("targets", targets, "Targets to install");

  install->callback([&] { command = make_unique<Install>(force, path, targets); });

  // Uninstall
  // TODO : add --project-path to uninstall all project dependencies

  uninstall->add_option("targets", targets, "Targets to uninstall");

  uninstall->callback([&] { command = make_unique<Uninstall>(force, targets); });

  // Update

  update->add_option("--path,-p", path, "Update local package from its root path");
  update->add_option("targets", targets, "Targets to update");

  update->callback([&] { command = make_unique<Update>(force, path, targets); });

  // Login

  login->callback([&] { command = make_unique<Login>(force); });

  // Logout

  logout->callback([&] { command = make_unique<Logout>(force); });

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
}
