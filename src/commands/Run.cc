#include <algorithm>
#include <filesystem>
#include <iostream>

#include "commands/Run.hh"
#include "files.hh"
#include "helpers.hh"
#include "objects/Controllers/Controller.hh"
#include "objects/Controllers/GlobalController.hh"
#include "objects/ZCError.hh"

using namespace std;
namespace fs = std::filesystem;

Run::Run(
    bool force, bool quiet, bool keep, bool plus, bool preprocess, bool compile, bool assemble, bool add_std,
    bool static_compile, const std::vector<std::string> &files, const std::vector<std::string> &args
)
    : Command(force, quiet), add_std_(add_std), keep_(keep), plus_(plus),
      mode_(getMode(preprocess, compile, assemble)), args_(args), static_(static_compile), g_(logger_, force)
{
  // Fill files_
  for (const auto &f : files)
    files_.emplace_back(f);

  // Check if C++ was given
  if (!plus_)
    plus_ = isCppAndCheckExtensions();
}

int Run::operator()()
{
  // 1. Check that all files exist (file extensions were already checked before)
  for (const auto &f : files_)
    if (!fs::exists(f))
      throw ZCError(ZC_NOT_FOUND, "File not found: " + escape_shell_arg(f.string()));

  // 2. Build the compiling command following the given options
  switch (mode_)
  {
  case Mode::PREPROCESS:
    output_name_ = files_[0].replace_extension(".i").string();
    break;
  case Mode::COMPILE:
    output_name_ = files_[0].replace_extension(".s").string();
    break;
  case Mode::ASSEMBLE:
    output_name_ = files_[0].replace_extension(".o").string();
    break;
  case Mode::FULL:
    output_name_ = files_[0].replace_extension().string();
    break;
  }

  if (fs::exists(output_name_) && !force_)
    if (!ask("The file '" + output_name_ + "' already exists. Do you want to overwrite it ?"))
      return 0;

  buildCommand();

#ifdef DEBUG_MODE
  logger_(LogLevel::DEBUG, build_cmd_);
#endif

  cout << flush;
  // TODO : get command output in a variable instead of stdout for better interface

  // 3. Compile program
  if (const int compile_res = system(build_cmd_.c_str()); compile_res != 0)
    throw ZCError(ZC_COMPILATION_ERROR, "Compilation failed");

  logger_(LogLevel::SUCCESS, "Compilation successful.");

  if (mode_ != Mode::FULL)
  {
    logger_(LogLevel::SUCCESS, "File created: " + output_name_);
    return 0;
  }

  // 4. Execute program
  if (g_.gc_->clear_before_run_)
  {
    if (const int clear_res = system("clear"); clear_res != 0)
      throw ZCError(ZC_INTERNAL_ERROR, "Unexpected terminal clearing error");
  }

  logger_(LogLevel::INFO, "Executing program...");
  string exec_cmd = fs::absolute(output_name_).string();

  for (const auto &arg : args_)
    exec_cmd += " " + escape_shell_arg(arg);

  const int run_res = system(exec_cmd.c_str());

  if (!g_.gc_->auto_keep_ && !keep_ && fs::exists(output_name_))
  {
    fs::remove(output_name_);
#ifdef DEBUG_MODE
    logger_(LogLevel::DEBUG, "Temporary file removed: " + output_name_);
#endif
  }

  if (run_res == 0)
    return 0;

  stringstream msg;
  msg << "Program exited with code " << run_res;
  throw ZCError(ZC_EXECUTION_ERROR, msg.str());
}

Mode Run::getMode(const bool preprocess, const bool compile, const bool assemble)
{
  int flags_found = 0;
  Mode mode(Mode::FULL);
  if (preprocess)
  {
    flags_found++;
    mode = Mode::PREPROCESS;
  }
  if (compile)
  {
    flags_found++;
    mode = Mode::COMPILE;
  }
  if (assemble)
  {
    flags_found++;
    mode = Mode::ASSEMBLE;
  }
  if (flags_found > 1)
    throw ZCError(ZC_INCOMPATIBLE_FLAGS, "Incompatible options");
  return mode;
}

bool Run::isCppAndCheckExtensions() const
{
  bool found = false;
  for (const auto &f : files_)
  {
    if (isCpp(f))
    {
      found = true;
      continue;
    }
    auto ext = f.extension().string();
    if (ext == ".c" || ext == ".i" || ext == ".o" || ext == ".s" || ext == ".asm")
      continue;
    throw ZCError(ZC_UNSUPPORTED_LANGUAGE, "File has an unknown extension: " + f.string());
  }
  return found;
}

void Run::buildCommand()
{
  stringstream cmd;
  // Compiler and standard
  if (plus_)
  {
    cmd << g_.gc_->cpp_compiler_ << " ";
    if (g_.gc_->add_std_ || add_std_)
      cmd << "'-std=" << g_.gc_->cpp_std_ << "' ";
  }
  else
  {
    cmd << g_.gc_->c_compiler_ << " ";
    if (g_.gc_->add_std_ || add_std_)
      cmd << "'-std=" << g_.gc_->c_std_ << "' ";
  }

  // User flags
  for (const auto &f : g_.gc_->flags_)
    cmd << escape_shell_arg(f) << " ";

  // Source files
  for (const auto &file : files_)
    cmd << escape_shell_arg(file.string()) << " ";

  // Output
  cmd << "-o " << escape_shell_arg(output_name_) << " ";

  // Mode and libraries for normal mode
  switch (mode_)
  {
  case Mode::PREPROCESS:
    cmd << "-E ";
    break;
  case Mode::COMPILE:
    cmd << "-S ";
    break;
  case Mode::ASSEMBLE:
    cmd << "-c ";
    break;
  default:
    cmd << "-I" << g_.include_dir_.string() << " "; // Path to headers
    for (const vector<string> libs = getInclusions(); const auto lib : libs)
    {
      fs::path lib_dir = (g_.lib_dir_ / lib);
      if (fs::exists(lib_dir))
      {
        cmd << "-L" << escape_shell_arg(lib_dir.string()) << " "; // Path to libraries
        cmd << "-Wl,-rpath," << escape_shell_arg(lib_dir.string()) << " ";
      }
      cmd << "-l" << lib << " ";
    }
    break;
  }

  // Color flags
  if (static_)
    cmd << "-static ";
  cmd << "-fdiagnostics-color=always";
  build_cmd_ = cmd.str();
}

vector<string> Run::getInclusions() const
{
  vector<string> libs_to_link;

  for (const auto &f : files_)
  {
    for (vector<string> includes = getFileInclusions(f, g_.r_->getPackages()); const auto &include : includes)
    {
      const bool already_present =
          std::any_of(libs_to_link.begin(), libs_to_link.end(), [&](const auto &p) { return p == include; });

      if (!already_present)
        libs_to_link.push_back(include);
    }
  }
  return libs_to_link;
}
