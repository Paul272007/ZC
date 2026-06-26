#include "Run.h"

#include <filesystem>
#include <string>
#include <vector>

#include "../clang_utils.h"
#include "../helpers.h"
#include "commands/Command.h"
#include "CompileMode.h"
#include "config/LanguageConf.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "Language.h"

ZC_DEV_CONFIG

namespace zc
{

Run::Run(
  const bool force, const std::vector<std::string> &files, const std::vector<std::string> &args,
  const bool preprocess, const bool compile, const bool assemble, const bool plus, const bool keep,
  const bool add_std, const bool static_link, const bool no_flags
)
  : Command(force),
    files_(str_to_path(files)),
    args_(args),
    add_flags_(!no_flags),
    plus_(plus || has_cpp()),
    add_std_(add_std),
    keep_(keep),
    static_(static_link)
{
  mode_ = parse_mode<CompileMode>(
    {
      { CompileMode::preprocess, preprocess },
      { CompileMode::compile, compile },
      { CompileMode::assemble, assemble },
    },
    CompileMode::full
  );
  output_name_ = get_output_name();
  build_cmd_   = get_build_command();
}

void Run::operator()()
{
  for (const auto &f : files_)
    if (!fs::exists(f))
      throw ZCException(ZCE_NOT_FOUND, "File not found: " + f.string());

  if (fs::exists(output_name_) && !force_)
    if (!if_.ask("The file '" + output_name_ + "' already exists. Do you want to overwrite it ?"))
      throw ZCException(ZCE_ABORTED, "Compilation aborted.");

#ifdef DEBUG_MODE
  if_.debug(build_cmd_);
#endif

  // TODO : capture build command output for better ui

  if (system(build_cmd_.c_str()) != 0)
    throw ZCException(ZCE_COMPILATION_ERROR, "Compilation failed");

  if_.success("Compilation successful.");

  if (mode_ != CompileMode::full)
  {
    if_.success("File created: " + output_name_);
    return;
  }

  if (gc_.clear_before_run)
    if_.clear();

  if_.info("Executing program...");
  string exec_cmd = escape_shell_arg(fs::absolute(output_name_).string());

#ifdef DEBUG_MODE
  if_.debug(exec_cmd);
#endif

  for (const auto &arg : args_)
    exec_cmd += " " + escape_shell_arg(arg);

  const int run_res = system(exec_cmd.c_str());

  if (!gc_.always_keep && !keep_ && fs::exists(output_name_))
  {
    fs::remove(output_name_);
#ifdef DEBUG_MODE
    if_.debug("Temporary file removed: " + output_name_);
#endif
  }

  if (run_res != 0)
    throw ZCException(ZCE_RUNTIME_ERROR, "Program exited with code " + to_string(run_res));
}

bool Run::has_cpp()
{
  for (const auto &f : files_)
    if (is_of_language(CXX, f))
      return true;
  return false;
}

std::string Run::get_build_command()
{
  stringstream cmd;

  // Compiler and standard
  LanguageConf lc;
  if (plus_)
    lc = gc_.get_lang_conf(CXX);
  else
    lc = gc_.get_lang_conf(C);

  cmd << escape_shell_arg(lc.compiler) << " ";
  if (gc_.always_add_std || add_std_)
    cmd << "'-std=" << lc.std << "' ";

  // User flags
  if (add_flags_)
    for (const auto &flag : lc.flags)
      cmd << escape_shell_arg(flag) << " ";

  // Source files
  for (const auto &file : files_)
    cmd << escape_shell_arg(file.string()) << " ";

  // Output
  cmd << "-o " << escape_shell_arg(output_name_) << " ";

  // Include directories
  cmd << "-I" << zc_root() / INCLUDE_DIR << " ";

  // Mode and libraries for normal mode
  switch (mode_)
  {
  case zc::CompileMode::preprocess:
    cmd << "-E ";
    break;
  case zc::CompileMode::compile:
    cmd << "-S ";
    break;
  case zc::CompileMode::assemble:
    cmd << "-c ";
    break;
  case zc::CompileMode::full: // Else link libraries
  default:
    for (const auto &lib : get_dependencies())
    {
      const string target = rg_.get_pkg(lib.name).target;
      if (lib.origin != "std")
      {
        fs::path lib_dir = zc_root() / LIB_DIR / lib.name;
        if (fs::exists(lib_dir))
        {
          cmd << "-L" << lib_dir << " ";
          cmd << "-Wl,-rpath," << lib_dir << " ";
        }
      }
      cmd << "-l" << escape_shell_arg(target) << " ";
    }
    break;
  }

  if (static_)
    cmd << "-static ";

  cmd << "-fdiagnostics-color=always";

  if (if_.is_quiet())
    cmd << HIDE_OUTPUT;

  return cmd.str();
}

string Run::get_output_name()
{
  fs::path out = files_[0];
  switch (mode_)
  {
  case CompileMode::preprocess:
    return out.replace_extension(".i").string();
  case CompileMode::compile:
    return out.replace_extension(".s").string();
  case CompileMode::assemble:
    return out.replace_extension(".o").string();
  case CompileMode::full:
  default:
    return out.replace_extension().string();
  }
}

vector<Dependency> Run::get_dependencies()
{
  vector<Dependency> libs_to_link;

  for (const auto &f : files_)
  {
    for (const auto &include : get_file_includes(f, rg_.pkgs()))
    {
      const bool already_present =
        std::any_of(libs_to_link.begin(), libs_to_link.end(), [&](const auto &p) { return p == include; });

      if (!already_present)
        libs_to_link.push_back(include);
    }
  }
  return libs_to_link;
}

} // namespace zc
