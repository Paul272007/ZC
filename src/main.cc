#include <exception>
#include <memory>
#include <string>
#include <vector>

#include <CLI11.hpp>
#include <commands/Command.hh>
#include <commands/Create.hh>
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

  bool force = false;
  bool quiet = false;
  vector<string> input_files;
  vector<string> output_files;

  // zc run
  bool run_keep = false;
  bool run_plus = false;
  bool run_c = false;
  bool run_S = false;
  bool run_E = false;
  vector<string> run_args;

  // zc create
  bool edit = false;

  /* ========================================================= *
   *                         SUBCOMMANDS                       *
   * ========================================================= */

  // clang-format off
  const auto run    = app.add_subcommand("run", "Compile and execute C/C++ file(s)");
  const auto create = app.add_subcommand("create", "Create file based on template");

  // ========================== RUN ===============================

  run->add_option("files", input_files, "The files to be compiled (and executed)")->required();
  run->add_option("--args,-a", run_args, "Arguments to be passed to the program when executing");

  run->add_flag("--keep,-k", run_keep, "Do not delete the executable after program ends");
  run->add_flag("--plus,-p", run_plus, "Force compilation as C++");
  run->add_flag("-E", run_E, "Preprocess only");
  run->add_flag("-S", run_S, "Compile, but do not assemble or link");
  run->add_flag("-c", run_c, "Compile and assemble, but do not link");
  run->add_flag("--quiet,-q", quiet, "Enable quiet mode");

  run->callback([&]() { command = make_unique<Run>(input_files, run_args, run_keep, run_plus, run_E, run_S, run_c, quiet); });

  // ========================== CREATE ===============================
  
  create->add_option("files", output_files, "The files to be created")->required();
  create->add_option("--input,-i", input_files, "Files to be used as basis to write the new files");

  create->add_flag("--force,-f", force, "Force writing into the files even if they already exist");
  create->add_flag("--edit,-e", edit, "Edit the files once created");

  create->callback([&]() { command = make_unique<Create>(output_files, force, input_files, edit); });

  /* ========================================================= *
   *                          PARSING                          *
   * ========================================================= */

  // clang-format on
  try
  {
    app.parse(argc, argv);
    if (command)
      return command->execute();
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
