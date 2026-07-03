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

#include <memory>
#include <vector>

#include "CLI11.h"
#include "commands/Add.h"
#include "commands/Build.h"
#include "commands/Clean.h"
#include "commands/Command.h"
#include "commands/Config.h"
#include "commands/Create.h"
#include "commands/Init.h"
#include "commands/Install.h"
#include "commands/Languages/LanguagesAdd.h"
#include "commands/Languages/LanguagesEdit.h"
#include "commands/Languages/LanguagesRemove.h"
#include "commands/Languages/LanguagesShow.h"
#include "commands/List.h"
#include "commands/Login.h"
#include "commands/Logout.h"
#include "commands/Publish.h"
#include "commands/Remove.h"
#include "commands/Run.h"
#include "commands/Setup.h"
#include "commands/Uninstall.h"
#include "commands/Update.h"
#include "commands/Use.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "ui/Interface.h"
#include "ui/ui_utils.h"

#define ZC_DEV_MAJOR   0
#define ZC_DEV_MINOR   1
#define ZC_DEV_PATCH   1
#define ZC_DEV_VERSION "0.1.1"

ZC_DEV_CONFIG

using namespace zc;

int main(const int argc, char *argv[])
{
  // --- Initialize app
  CLI::App app("ZC utility for C and C++");
  argv = app.ensure_utf8(argv);
  app.set_version_flag("--version,-v", "ZC v" ZC_DEV_VERSION " (unstable)");
  app.require_subcommand(1);
  unique_ptr<Command> command(nullptr);

  // --- Command line arguments and flags as variables
  bool quiet = false;
  bool force = false;
  // Run
  bool preprocess  = false;
  bool compile     = false;
  bool assemble    = false;
  bool plus        = false;
  bool keep        = false;
  bool std         = false;
  bool static_link = false;
  bool no_flags    = false;
  // Build
  bool clean_before = false;
  bool debug        = false;
  bool release      = false;
  int  jobs         = 1;
  // Init
  bool git        = false;
  bool edit       = false;
  bool is_bin     = false;
  bool is_lib     = false;
  bool is_header  = false;
  bool is_compose = false;
  // List
  bool show_deps        = false;
  bool show_templates   = false;
  bool show_p_templates = false;
  bool simple_display   = false;
  bool show_remote      = false;
  // Update
  bool sync     = false;
  bool dont_use = false;
  // Use
  bool global = false;

  string author;
  string target;
  string p_template;
  string name;
  string path;
  string p_root;

  string key;
  string value;

  vector<string> run_args;
  vector<string> targets;
  vector<string> input_files;
  vector<string> langs;

  // clang-format off

  // --- Subcommands
  // Files
  auto *const run       = app.add_subcommand("run",       "Compile and execute C/C++ file(s)");
  auto *const create    = app.add_subcommand("create",    "Create file based on template");
  // Projects
  auto *const init      = app.add_subcommand("init",      "Initialize empty project");
  auto *const setup     = app.add_subcommand("setup",     "(Re)generate build configuration");
  auto *const build     = app.add_subcommand("build",     "Build project");
  auto *const add       = app.add_subcommand("add",       "Add dependencies to project");
  auto *const remove    = app.add_subcommand("remove",    "Remove dependencies from project");
  auto *const use       = app.add_subcommand("use",       "Choose version of a package to use");
  auto *const publish   = app.add_subcommand("publish",   "Publish package to server");
  auto *const clean     = app.add_subcommand("clean",     "Clean all temporary files and directories");
  // Packages
  auto *const list      = app.add_subcommand("list",      "List installed libraries");
  auto *const install   = app.add_subcommand("install",   "Install packages");
  auto *const uninstall = app.add_subcommand("uninstall", "Uninstall packages");
  auto *const update    = app.add_subcommand("update",    "Update packages");
  // Configuration
  auto *const login     = app.add_subcommand("login",     "Log into an account");
  auto *const logout    = app.add_subcommand("logout",    "Log out");
  auto *const config    = app.add_subcommand("config",    "Change configuration");

  auto *const languages = app.add_subcommand("languages", "Manage languages")->require_subcommand();
  auto *const languages_add     = languages->add_subcommand("add",    "Add languages");
  auto *const languages_edit    = languages->add_subcommand("edit",   "Edit languages");
  auto *const languages_remove  = languages->add_subcommand("remove", "Remove languages");
  auto *const languages_show    = languages->add_subcommand("show",   "Show languages");

  // --- Subcommands arguments
  // Run

  run->add_option("files", targets, "Files to compile and run");
  run->add_option("--args,-a", run_args, "Arguments to be passed to the program when executed");
  run->add_option("--project-path,-P", p_root, "Directory to use as project root");

  run->add_flag("--quiet,-q", quiet, "Do not show any messages");
  run->add_flag("--force,-f", force, "Force compiling even if target already exists");
  run->add_flag("--preprocess,-E", preprocess, "Preprocess only");
  run->add_flag("--compile,-c", compile, "Compile and assemble, but do not link");
  run->add_flag("--assemble,-S", assemble, "Compile, but do not assemble or link");
  run->add_flag("--plus,-p", plus, "Force compilation as C++");
  run->add_flag("--keep,-k", keep, "Do not delete the executable after program ends");
  run->add_flag("--std", std, "Add C/C++ standard from config file");
  run->add_flag("--static,-s", static_link, "Compile prioritizing the use of static libraries");
  run->add_flag("--no-flags,-n", no_flags, "Do not add flags from configuration file");
  run->add_flag("--release,-r", release, "Compile in release mode");

  run->callback([&] { command = make_unique<Run>(force, targets, run_args, p_root, preprocess, compile, assemble, plus, keep, std, static_link, no_flags, release); });

  // Create

  create->add_option("files", targets, "Files to create")->required();
  create->add_option("--input,-i", input_files, "Files to use as input for the new files");

  create->add_flag("--force,-f", force, "Force creating file even if it already exists");
  create->add_flag("--edit,-e", edit, "Open files in editor once created");

  create->callback([&] {command = make_unique<Create>(force, edit, targets, input_files);});

  // Init

  init->add_option("--project-path,-P", p_root, "Directory to use as project root");
  init->add_option("--author,-a", author, "Package author");
  init->add_option("--target,-t", target, "Package target");
  init->add_option("--project-template,-p", p_template, "Project template to use");
  init->add_option("--name,-n", name, "Name of the package");
  init->add_option("--languages,-l", langs, "Languages of the project");

  init->add_flag("--quiet,-q", quiet, "Do not show any messages");
  init->add_flag("--force,-f", force, "Force initialization even if a project already exists");
  init->add_flag("--git,-g", git, "Initialize empty git repository at project root");
  init->add_flag("--edit,-e", edit, "Open project in editor once initialized");
  init->add_flag("--bin,-B", is_bin, "Make package of type BIN");
  init->add_flag("--lib,-L", is_lib, "Make package of type LIB");
  init->add_flag("--header,-H", is_header, "Make package of type HEADER");
  init->add_flag("--compose,-C", is_compose, "Make package of type COMPOSE");

  init->callback([&] { command = make_unique<Init>(force, p_root, git, edit, author, target, p_template, name, is_bin, is_lib, is_header, is_compose, langs); });

  // Setup
  // TODO: add --force,-f flag

  setup->add_option("--project-path,-P", p_root, "Directory to use as project root");

  setup->add_flag("--quiet,-q", quiet, "Do not show any messages");
  setup->add_flag("--release,-r", release, "Create config for release mode");
  setup->add_flag("--debug,-d", debug, "Create config for debug mode");

  setup->callback([&] { command = make_unique<Setup>(force, p_root, release, debug); });

  // Build
  // TODO: add --force,-f flag

  build->add_option("--project-path,-P", p_root, "Directory to use as project root");
  auto *opt_jobs = build->add_option("--jobs,-j", jobs, "Number of concurrent jobs for compilation")->expected(0, 1);
  auto *opt_args = build->add_option("--run,-R", run_args, "Run binary after compiling and optionally add parameters")->expected(0, -1);

  build->add_flag("--quiet,-q", quiet, "Do not show any messages");
  build->add_flag("--clean,-c", clean_before, "Clean before building");
  build->add_flag("--release,-r", release, "Build in release mode");
  build->add_flag("--debug,-d", debug, "Build in debug mode");

  build->callback([&] {
    command = make_unique<Build>(force, p_root, clean_before, release, debug, static_cast<bool>((*opt_args)), run_args, static_cast<bool>((*opt_jobs)), jobs);
  });

  // Add
  // TODO: add --force,-f flag

  add->add_option("--project-path,-P", p_root, "Directory to use as project root");
  add->add_option("targets", targets, "The dependencies to be added")->required();

  add->add_flag("--quiet,-q", quiet, "Do not show any messages");
  add->add_flag("--static,-s", static_link, "Add dependency as static library instead of shared library");

  add->callback([&] { command = make_unique<Add>(force, p_root, targets, static_link); });

  // Remove
  // TODO: add --force,-f flag

  remove->add_option("--project-path,-P", p_root, "Directory to use as project root");
  remove->add_option("targets", targets, "The dependencies to be removed")->required();

  remove->add_flag("--quiet,-q", quiet, "Do not show any messages");

  remove->callback([&] { command = make_unique<Remove>(force, p_root, targets); });

  // Use
  // TODO: add --force,-f flag

  use->add_option("--project-path,-P", p_root, "Directory to use as project root");
  use->add_option("targets", targets, "The dependencies and their version to use")->required();

  use->add_flag("--quiet,-q", quiet, "Do not show any messages");
  use->add_flag("--global,-g", global, "Change default version of a package");

  use->callback([&] { command = make_unique<Use>(force, p_root, targets, global); });

  // Publish
  // TODO: add --force,-f flag

  publish->add_option("--project-path,-P", p_root, "Directory to use as project root");

  publish->add_flag("--quiet,-q", quiet, "Do not show any messages");

  publish->callback([&] { command = make_unique<Publish>(force, p_root); });

  // Clean
  // TODO: add --force,-f flag

  clean->add_option("--project-path,-P", p_root, "Directory to use as project root");

  clean->add_flag("--quiet,-q", quiet, "Do not show any messages");

  clean->callback([&] { command = make_unique<Clean>(force, p_root); });

  // List
  // TODO: add --force,-f flag
  list->add_option("--project-path,-P", p_root, "Directory to use as project root");

  list->add_flag("--quiet,-q", quiet, "Do not show any messages");
  list->add_flag("--dependencies,-d", show_deps, "Show project dependencies");
  list->add_flag("--templates,-t", show_templates, "Show available templates instead of packages");
  list->add_flag("--project-templates,-p", show_p_templates, "Show available project templates instead of packages");
  list->add_flag("--remote,-r", show_remote, "Show remote packages instead of local ones");
  list->add_flag("--simple,-s", simple_display, "Use a simpler display");

  list->callback([&] { command = make_unique<List>(force, p_root, show_deps, show_templates, show_p_templates, show_remote, simple_display); });

  // Install

  install->add_option("--project-path,-P", p_root, "Directory to use as project root");
  install->add_option("--path,-p", path, "Install from local project instead of remote");
  install->add_option("targets", targets, "Targets to install");

  install->add_flag("--quiet,-q", quiet, "Do not show any messages");
  install->add_flag("--force,-f", force, "Force reinstalling packages");
  install->add_flag("--sync,-s", sync, "Also add installed packages to project dependencies");
  install->add_flag("--std", std, "Add dependency to a standard library instead of ZC library");

  install->callback([&] { command = make_unique<Install>(force, p_root, path, targets, sync, std); });

  // Uninstall
  // TODO: add --force,-f flag

  uninstall->add_option("--project-path,-P", p_root, "Directory to use as project root");
  uninstall->add_option("targets", targets, "Targets to uninstall");

  uninstall->add_flag("--quiet,-q", quiet, "Do not show any messages");

  uninstall->callback([&] { command = make_unique<Uninstall>(force, p_root, targets); });

  // Update

  update->add_option("--project-path,-P", p_root, "Directory to use as project root");
  update->add_option("--path,-p", path, "Update local package from its root path");
  update->add_option("targets", targets, "Targets to update");

  update->add_flag("--quiet,-q", quiet, "Do not show any messages");
  update->add_flag("--force,-f", force, "Force reinstalling a specific version");
  update->add_flag("--sync,-s", sync, "Sync project dependencies after updating packages");
  update->add_flag("--dont-use,-d", dont_use, "Do not set newly installed version as default version for updated package");

  update->callback([&] { command = make_unique<Update>(force, p_root, path, targets, sync, dont_use); });

  // Login

  login->add_flag("--quiet,-q", quiet, "Do not show any messages");
  login->add_flag("--force,-f", force, "Force login even if an account is already logged in");

  login->callback([&] { command = make_unique<Login>(force); });

  // Logout
  // TODO: add --force,-f flag

  logout->add_flag("--quiet,-q", quiet, "Do not show any messages");

  logout->callback([&] { command = make_unique<Logout>(force); });

  // Config

  config->add_option("key", key, "Key to modify")->required();
  config->add_option("value", value, "Value to give to the key (default: restore to default option)");

  config->add_flag("--quiet,-q", quiet, "Do not show any messages");
  config->add_flag("--force,-f", force, "When creating a new config, override already existing configuration");

  config->callback([&] { command = make_unique<Config>(force, key, value); });

  // Languages Add
  // TODO: add --force,-f flag

  languages_add->add_option("--project-path,-P", p_root, "Directory to use as project root");
  languages_add->add_option("languages", langs, "Languages to add")->required();

  languages_add->add_flag("--global,-g", global, "Modify global configuration");

  languages_add->callback([&] { command = make_unique<LanguagesAdd>(force, p_root, langs, global); });

  // Languages Edit
  // TODO: add --force,-f flag

  languages_edit->add_option("--project-path,-P", p_root, "Directory to use as project root");
  languages_edit->add_option("languages", langs, "Languages to edit");

  languages_edit->add_flag("--global,-g", global, "Modify global configuration");

  languages_edit->callback([&] { command = make_unique<LanguagesEdit>(force, p_root, langs, global); });

  // Languages Remove
  // TODO: add --force,-f flag

  languages_remove->add_option("--project-path,-P", p_root, "Directory to use as project root");
  languages_remove->add_option("languages", langs, "Languages to remove")->required();

  languages_remove->add_flag("--global,-g", global, "Modify global configuration");

  languages_remove->callback([&] { command = make_unique<LanguagesRemove>(force, p_root, langs, global); });

  // Languages Show
  // TODO: add --force,-f flag

  languages_show->add_option("--project-path,-P", p_root, "Directory to use as project root");
  languages_show->add_option("languages", langs, "Languages to show");

  languages_show->add_flag("--global,-g", global, "Show global configuration");

  languages_show->callback([&] { command = make_unique<LanguagesShow>(force, p_root, langs, global); });

  // clang-format on

  try
  {
    app.parse(argc, argv);
    Interface::get().set_quiet(quiet);
    if (command)
      (*command)();
    return 0;
  }
  catch (const CLI::ParseError &e)
  {
    return app.exit(e);
  }
  catch (const ZCException &e)
  {
    cerr << e << '\n';
    return e.code();
  }
  catch (const exception &e)
  {
    cerr << RED << "Unexpected error: " << RESET << e.what() << '\n';
    return 255;
  }
}
