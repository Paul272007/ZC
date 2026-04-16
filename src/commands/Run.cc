#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

#include <commands/Command.hh>
#include <commands/Run.hh>
#include <helpers.hh>
#include <interface.hh>
#include <objects/File.hh>
#include <objects/Settings.hh>
#include <objects/ZCError.hh>

using namespace std;
namespace fs = std::filesystem;

Run::Run(
    const std::vector<std::string> &files, const std::vector<std::string> &args, const bool keep,
    const bool plus, const bool preprocess, const bool compile, const bool assemble, const bool force,
    const bool quiet, const bool add_std
)
    : Command(force, quiet), settings_(Settings::getInstance()), registry_(Registry(true)), add_std_(add_std),
      keep_(keep), plus_(plus), mode_(getMode(preprocess, compile, assemble)), args_(args)
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
    if (!f.exists())
      throw ZCError(ZC_NOT_FOUND, "File not found: " + escape_shell_arg(f.getPath().string()));

  // 2. Build the compiling command following the given options
  switch (mode_)
  {
  case PREPROCESS:
    output_name_ = files_[0].getPath().replace_extension(".i").string();
    break;
  case COMPILE:
    output_name_ = files_[0].getPath().replace_extension(".s").string();
    break;
  case ASSEMBLE:
    output_name_ = files_[0].getPath().replace_extension(".o").string();
    break;
  case FULL:
    output_name_ = files_[0].getPath().replace_extension().string();
    break;
  }

  if (fs::exists(output_name_) && !force_)
    if (!ask("The file '" + output_name_ + "' already exists. Do you want to overwrite it ?"))
      return 0;

  buildCommand();

#ifdef DEBUG_MODE
  if (!quiet_)
    debug(build_cmd_);
#endif

  cout << flush;
  // TODO : get command output in a variable instead of stdout for better interface

  // 3. Compile program
  if (const int compile_res = system(build_cmd_.c_str()); compile_res != 0)
    throw ZCError(ZC_COMPILATION_ERROR, "Compilation failed");

  if (!quiet_)
    success("Compilation successful.");

  if (mode_ != FULL)
  {
    if (!quiet_)
      success("File created: " + output_name_);
    return 0;
  }

  // 4. Execute program
  if (settings_.clear_before_run_)
  {
    if (const int clear_res = system("clear"); clear_res != 0)
      throw ZCError(ZC_INTERNAL_ERROR, "Unexpected terminal clearing error");
  }

  if (!quiet_)
    info("Executing program...");
  string exec_cmd = fs::absolute(output_name_).string();

  for (const auto &arg : args_)
    exec_cmd += " " + escape_shell_arg(arg);

  const int run_res = system(exec_cmd.c_str());

  if (!settings_.auto_keep_ && !keep_ && fs::exists(output_name_))
  {
    fs::remove(output_name_);
#ifdef DEBUG_MODE
    if (!quiet_)
    {
      cout << endl;
      debug("Temporary file removed: " + output_name_);
    }
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
  Mode mode(FULL);
  if (preprocess)
  {
    flags_found++;
    mode = PREPROCESS;
  }
  if (compile)
  {
    flags_found++;
    mode = COMPILE;
  }
  if (assemble)
  {
    flags_found++;
    mode = ASSEMBLE;
  }
  if (flags_found > 1)
    throw ZCError(ZC_INCOMPATIBLE_FLAGS, "Incompatible options");
  return mode;
}

bool Run::isCppAndCheckExtensions() const
{
  bool found = false;
  for (const auto &f : files_)
    switch (f.getLanguage())
    {
    case CPP:
      found = true;
      break;
    case C:
    case OBJECT:
    case ASSEMBLER:
    case INSTANCE:
      break;
    default:
      throw ZCError(ZC_UNSUPPORTED_LANGUAGE, "File has an unknown extension: " + f.getPath().string());
    }
  return found;
}

void Run::buildCommand()
{
  stringstream cmd;
  // Compiler and standard
  if (plus_)
  {
    cmd << settings_.cpp_compiler_ << " ";
    if (settings_.auto_add_std_ || add_std_)
      cmd << "'-std=" << settings_.cpp_std_ << "' ";
  }
  else
  {
    cmd << settings_.c_compiler_ << " ";
    if (settings_.auto_add_std_ || add_std_)
      cmd << "'-std=" << settings_.c_std_ << "' ";
  }

  // User flags
  for (const auto &f : settings_.flags_)
    cmd << escape_shell_arg(f) << " ";

  // Source files
  for (const auto &file : files_)
    cmd << file << " ";

  // Output
  cmd << "-o " << escape_shell_arg(output_name_) << " ";

  // Mode and libraries for normal mode
  switch (mode_)
  {
  case PREPROCESS:
    cmd << "-E ";
    break;
  case COMPILE:
    cmd << "-S ";
    break;
  case ASSEMBLE:
    cmd << "-c ";
    break;
  default:
    cmd << "-I" << registry_.getIncludePath().string() << " "; // Path to headers
    for (const vector<string> libs = getInclusions(); const auto lib : libs)
    {
      fs::path lib_dir = (registry_.getLibPath() / lib);
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
  cmd << "-fdiagnostics-color=always";
  build_cmd_ = cmd.str();
}

vector<string> Run::getInclusions() const
{
  vector<string> libs_to_link;

  for (const auto &f : files_)
  {
    for (vector<string> includes = f.getInclusions(registry_); const auto &include : includes)
    {
      const bool already_present =
          std::any_of(libs_to_link.begin(), libs_to_link.end(), [&](const auto &p) { return p == include; });

      if (!already_present)
        libs_to_link.push_back(include);
    }
  }
  return libs_to_link;
}
