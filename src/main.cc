#include <exception>
#include <memory>
#include <string>
#include <vector>

#include <CLI11.hpp>
#include <commands/Add.hh>
#include <commands/Build.hh>
#include <commands/Command.hh>
#include <commands/Create.hh>
#include <commands/Init.hh>
#include <commands/Install.hh>
#include <commands/List.hh>
#include <commands/Publish.hh>
#include <commands/Remove.hh>
#include <commands/Run.hh>
#include <interface.hh>
#include <objects/ZCError.hh>

using namespace std;

int main(const int argc, char *argv[])
{
  // Initialize app
  CLI::App app("ZC utility for C and C++");
  argv = app.ensure_utf8(argv);
  app.set_version_flag("--version,-v", "ZC v0.0.1 (unstable)");
  app.require_subcommand(1);
  unique_ptr<Command> command(nullptr);

  /* ========================================================= *
   *                  VARIABLES FOR ARGUMENTS                  *
   * ========================================================= */

  // For more than one command
  bool force = false;
  bool quiet = false;
  bool edit = false;
  bool git = false;
  bool global = false;
  string path;
  vector<string> input_files;
  vector<string> output_files;
  vector<string> targets;

  // zc run
  bool run_keep = false;
  bool run_plus = false;
  bool run_std = false;
  bool run_c = false;
  bool run_S = false;
  bool run_E = false;
  bool std = false;
  bool static_compile = false;
  vector<string> run_args;

  // zc init
  string author;
  string project_template;
  string name;
  string src_folder;
  string include_folder;
  string type;

  // zc build
  bool release_mode = false;

  /* ========================================================= *
   *                         SUBCOMMANDS                       *
   * ========================================================= */

  // clang-format off
  const auto run     = app.add_subcommand("run",     "Compile and execute C/C++ file(s)");
  const auto create  = app.add_subcommand("create",  "Create file based on template");
  const auto init    = app.add_subcommand("init",    "Initialize empty project");
  const auto list    = app.add_subcommand("list",    "List all globally installed libraries");
  const auto build   = app.add_subcommand("build",   "Build project");
  const auto install = app.add_subcommand("install", "Install package");
  const auto remove  = app.add_subcommand("remove",  "Remove package");
  const auto add     = app.add_subcommand("add",     "Add dependency from global registry");
  const auto publish = app.add_subcommand("publish", "Publish package to server");

  // ========================== RUN ===============================

  run->add_option("files", input_files, "The files to be compiled (and executed)")->required();
  run->add_option("--args,-a", run_args, "Arguments to be passed to the program when executed");

  run->add_flag("--keep,-k", run_keep, "Do not delete the executable after program ends");
  run->add_flag("--plus,-p", run_plus, "Force compilation as C++");
  run->add_flag("--quiet,-q", quiet, "Enable quiet mode");
  run->add_flag("--force,-f", force, "Force compiling even if target already exists");
  run->add_flag("--std", std, "Add C/C++ standard from config file");
  run->add_flag("--static,-s", static_compile, "Compile using static libraries");
  run->add_flag("-E", run_E, "Preprocess only");
  run->add_flag("-S", run_S, "Compile, but do not assemble or link");
  run->add_flag("-c", run_c, "Compile and assemble, but do not link");

  run->callback([&]() { command = make_unique<Run>(input_files, run_args, run_keep, run_plus, run_E, run_S, run_c, force, quiet, std, static_compile); });

  // ========================== CREATE ===============================
  
  create->add_option("files", output_files, "The files to be created")->required();
  create->add_option("--input,-i", input_files, "Files to be used as basis to write the new files");

  create->add_flag("--force,-f", force, "Force writing into the files even if they already exist");
  create->add_flag("--quiet,-q", quiet, "Do not show any messages");
  create->add_flag("--edit,-e", edit, "Edit the files once created");

  create->callback([&]() { command = make_unique<Create>(output_files, force, quiet, input_files, edit); });

  // ========================== INIT ===============================

  init->add_option("--author,-a", author, "Specify the project's author");
  init->add_option("--template,-t", project_template, "Specify template to use as basis");
  init->add_option("--name,-n", name, "Specify package name");
  init->add_option("--src-folder,-s", src_folder, "Specify source folder");
  init->add_option("--include-folder,-i", include_folder, "Specify include folder");
  init->add_option("--type,-T", type, "Specify package type");

  init->add_flag("--force,-f", force, "Force creating project even if one was already created in selected directory");
  init->add_flag("--quiet,-q", quiet, "Do not show any messages");
  init->add_flag("--edit,-e", edit, "Open project in editor once initialized");
  init->add_flag("--git,-g", git, "Create empty git repository");

  init->callback([&]() { command = make_unique<Init>(author, project_template, name, src_folder, include_folder, force, quiet, edit, git, type); });

  // ========================== LIST ===============================

  // list->add_flag("--force,-f", force, "Force");
  list->add_flag("--quiet,-q", quiet, "Do not show any messages");

  list->callback([&]() { command = make_unique<List>(false, quiet); });

  // ========================== BUILD ===============================

  build->add_option("path", path, "Path to project to build");

  build->add_flag("--force,-f", force, "Force regenerating CMakeLists.txt");
  build->add_flag("--quiet,-q", quiet, "Do not show any messages");

  build->callback([&]() { command = make_unique<Build>(force, quiet, path); });

  // ========================== INSTALL ===============================

  install->add_option("targets", targets, "The packages to be installed");
  install->add_option("--path,-p", path, "Install from local path instead of remote");

  install->add_flag("--global,-g", global, "Install the package globally");
  install->add_flag("--force,-f", force, "Force installing the package even if already installed");
  install->add_flag("--quiet,-q", quiet, "Do not show any messages");

  install->callback([&]() { command = make_unique<Install>(targets, path, global, force, quiet); });

  // ========================== REMOVE ===============================

  remove->add_option("targets", targets, "The packages to be removed")->required();
  
  // remove->add_flag("--force,-f", force, "Force removing packages");
  remove->add_flag("--quiet,-q", quiet, "Do not show any messages");
  remove->add_flag("--global,-g", global, "Remove globally installed package");

  remove->callback([&]() { command = make_unique<Remove>(targets, false, quiet, global); });

  // ========================== ADD ===============================

  add->add_option("targets", targets, "The packages to be added")->required();

  add->add_flag("--quiet,-q", quiet, "Do not show any messages");

  add->callback([&]() { command = make_unique<Add>(targets, false, quiet); });

  // ========================== PUBLISH ===============================

  publish->add_flag("--quiet,-q", quiet, "Do not show any messages");

  publish->callback([&]() { command = make_unique<Publish>(false, quiet); });

  // clang-format on
  /* ========================================================= *
   *                          PARSING                          *
   * ========================================================= */

  try
  {
    app.parse(argc, argv);
    if (command)
      return (*command)();
  }
  catch (const CLI::ParseError &e)
  {
    return app.exit(e);
  }
  catch (const ZCError &e)
  {
    cerr << e << endl;
    return e.getCode_();
  }
  catch (exception &e)
  {
    cerr << RED << "Unexpected error: " << COLOR_RESET << e.what() << endl;
    return -1;
  }
}
