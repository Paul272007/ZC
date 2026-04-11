#include "commands/Command.hh"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

#include <commands/Run.hh>
#include <helpers.hh>
#include <interface.hh>
#include <objects/File.hh>
#include <objects/Registry.hh>
#include <objects/Settings.hh>
#include <objects/ZCError.hh>

using namespace std;
namespace fs = std::filesystem;

Run::Run(
    const std::vector<std::string> &files, const std::vector<std::string> &args, const bool keep,
    const bool plus, const bool preprocess, const bool compile, const bool assemble, const bool force,
    const bool quiet
)
    : keep_(keep), plus_(plus), Command(force, quiet), mode_(getMode(preprocess, compile, assemble)),
      settings_(Settings::getInstance()), registry_(Registry::getInstance()), args_(args)
{
  // 1. Fill files_
  for (const auto &f : files)
    files_.emplace_back(f);

  // 2. Check if CPP was given and that files have correct extensions
  fs::path badFile;

  // 3. Check if C++ was given
  if (!plus_)
    plus_ = isCppAndCheckExtensions(badFile);

  if (!badFile.empty())
    throw ZCError(ZC_UNSUPPORTED_LANGUAGE, "File has an unknown extension: " + badFile.string());
}

int Run::execute()
{
  // 1. Check that all files exist (file extensions were already checked before)
  if (fs::path badFile; !filesExist(badFile))
    throw ZCError(ZC_NOT_FOUND, "File not found: " + badFile.string());

  string output_name, build_cmd;

  // 2. Build the compiling command following the given options
  switch (mode_)
  {
  case PREPROCESS:
    output_name = files_[0].getPath().replace_extension(".i").string();
    break;
  case COMPILE:
    output_name = files_[0].getPath().replace_extension(".s").string();
    break;
  case ASSEMBLE:
    output_name = files_[0].getPath().replace_extension(".o").string();
    break;
  case FULL:
    // default:
    output_name = files_[0].getPath().replace_extension("").string();
    break;
  }

  if (fs::exists(output_name) && !force_)
    if (!ask("The file '" + output_name + "' already exists. Do you want to overwrite it ?"))
      return 0;

  build_cmd = buildCommand(output_name);

#ifdef DEBUG_MODE
  if (!quiet_)
    debug("Build command: " + build_cmd);
#endif

  cout << flush;
  // TODO : get command output in a variable instead of stdout for better
  // display

  // 3. Compile program

  if (const int compile_res = system(build_cmd.c_str()); compile_res != 0)
    throw ZCError(ZC_COMPILATION_ERROR, "Compilation failed");

  if (!quiet_)
    success("Compilation successful.");

  if (mode_ != FULL)
  {
    if (!quiet_)
      success("File created: " + output_name);
    return 0;
  }

  // 4. Execute program
  if (settings_.getClearBeforeRun())
  {
    if (const int clear_res = system("clear"); clear_res != 0)
      throw ZCError(ZC_INTERNAL_ERROR, "Unexpected terminal clearing error");
  }

  if (!quiet_)
    info("Executing program...");
  string exec_cmd = fs::absolute(output_name).string();

  for (const auto &arg : args_)
    exec_cmd += " " + escape_shell_arg(arg);

  const int run_res = system(exec_cmd.c_str());

  if (!settings_.getAutoKeep() && !keep_ && fs::exists(output_name))
  {
    fs::remove(output_name);
#ifdef DEBUG_MODE
    if (!quiet_)
    {
      cout << endl;
      debug("Temporary file removed: " + output_name);
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

bool Run::isCppAndCheckExtensions(std::filesystem::path &badFile) const
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
      badFile = f.getPath();
      return false;
    }
  return found;
}

string Run::buildCommand(const string &output_name) const
{
  stringstream cmd;
  // Compiler and standard
  if (plus_)
  {
    cmd << settings_.getCppCompiler() << " ";
    if (settings_.getAutoAddStd())
      cmd << "'-std=" << settings_.getCppStd() << "' ";
  }
  else
  {
    cmd << settings_.getCCompiler() << " ";
    if (settings_.getAutoAddStd())
      cmd << "'-std=" << settings_.getCStd() << "' ";
  }

  // User flags
  for (const auto &f : settings_.getFlags())
    cmd << escape_shell_arg(f) << " ";

  // On build mode : use map header -> lib provided by the registry
  if (mode_ == FULL)
  {
    cmd << "-I" << escape_shell_arg(registry_.getIncludeDir()) << " ";
    cmd << "-L" << escape_shell_arg(registry_.getLibDir()) << " ";
    cmd << "-Wl,-rpath," << escape_shell_arg(registry_.getLibDir()) << " ";
  }

  // Source files
  for (const auto &file : files_)
    cmd << file << " ";

  // Output
  cmd << "-o " << escape_shell_arg(output_name) << " ";

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
    const vector<string> includes = getInclusions();
    for (const auto &include : includes)
      cmd << escape_shell_arg(include) << " ";
    break;
  }

  // Color flags
  cmd << "-fdiagnostics-color=always";
  return cmd.str();
}

bool Run::filesExist(fs::path &badFile) const
{
  for (const auto &f : files_)
  {
    if (!f.exists())
    {
      badFile = f.getPath();
      return false;
    }
  }
  return true;
}

vector<string> Run::getInclusions() const
{
  vector<string> flags;

  for (const auto &f : files_)
  {
    for (vector<string> includes = f.getInclusions(registry_); const auto &include : includes)
      if (ranges::find(flags, include) == flags.end())
        flags.push_back(include);
  }
  return flags;
}
