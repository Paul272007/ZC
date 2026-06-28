#include "Project.h"

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

#include "../config/GConf.h"
#include "../helpers.h"
#include "config/Dependency.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "Language.h"
#include "pkgs/Network.h"
#include "pkgs/PkgType.h"
#include "pkgs/Registry.h"
#include "project/MakeVariable.h"
#include "Version.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

Project::Project(const std::filesystem::path &root)
  : root_dir(root),
    build_dir(root / BUILD_DIR),
    pconf(root / ZC_FILE),
    cache_dir_(root / PROJECT_CACHE_DIR),
    makefile_(build_dir / MAKEFILE)
{
}

void Project::build(BuildMode current_mode, bool is_install)
{
  if (pconf.type == PkgType::HEADER)
    return;

  fs::create_directories(build_dir);
  const string make_cmd = "make --no-print-directory -C " + build_dir.string() + " all";
  int          compiled = 0;
  get_sources();

  if (!is_install)
  {
    const fs::path build_mode_file = build_dir / BUILD_MODE_FILE;
    if (fs::exists(build_mode_file))
    {
      const BuildMode previous_mode = build_mode_from_str(read_file(build_mode_file));

      if (current_mode == BuildMode::automatic)
        current_mode = previous_mode;
      else if (current_mode != previous_mode)
      {
        if_.info("Build mode switched to " + build_mode_to_str(current_mode) + ". Forcing full rebuild...");
        clean();
        fs::create_directories(build_dir); // so we can write into the build_mode_file and it doesn't crash
      }
    }
    else // if build mode file doesn't exist and mode is set to automatic, default is debug
    {
      current_mode = BuildMode::debug;
    }
    write_file(build_mode_file, build_mode_to_str(current_mode));
    init_variables(current_mode == BuildMode::release);
    generate_compile_commands();
  }
  else
    init_variables(current_mode == BuildMode::release);

  generate_Makefile();

  int          to_compile  = 0;
  int          to_link     = 0;
  const string dry_run_cmd = make_cmd + " -n 2>/dev/null";
  FILE        *dry_pipe    = popen(dry_run_cmd.c_str(), "r");
  if (!dry_pipe)
    throw ZCException(ZCE_INTERNAL_ERROR, "Failed to run make");

  char buffer_dry[512];
  while (fgets(buffer_dry, sizeof(buffer_dry), dry_pipe) != nullptr)
  {
    string line(buffer_dry);
    if (line.find("ZC_COMPILE|") != string::npos)
      to_compile++;
    else if (line.find("ZC_BIN|") != string::npos || line.find("ZC_STATIC|") != string::npos ||
             line.find("ZC_SHARED|") != string::npos)
      to_link++;
  }
  pclose(dry_pipe);

  if (to_compile == 0 && to_link == 0)
  {
    if_.success("Project is already up to date! Nothing to do.");
    return;
  }
  if_.info(to_string(to_compile) + " file(s) to compile, " + to_string(to_link) + " target(s) to link");

  to_compile += pconf.type == PkgType::BIN ? 1 : (pconf.type == PkgType::LIB ? 2 : 0);

  FILE *pipe = popen((make_cmd + " 2>&1").c_str(), "r");
  if (!pipe)
    throw ZCException(ZCE_INTERNAL_ERROR, "Failed to run make");

  char buffer[1024];

  while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
  {
    string line(buffer);
    if (!line.empty() && line.back() == '\n')
      line.pop_back();

    if (line.starts_with("ZC_"))
    {
      constexpr int bar_width = 20;
      compiled++;
      int percent = (compiled * 100) / to_compile;
      if (percent > 100)
        percent = 100;

      string message;
      string target_name;

      if (string rest = line.substr(3); rest.starts_with("COMPILE|"))
      {
        message     = "Compiling object ";
        target_name = rest.substr(8);
      }
      else if (rest.starts_with("STATIC|"))
      {
        message     = "Linking static library ";
        target_name = rest.substr(7);
      }
      else if (rest.starts_with("SHARED|"))
      {
        message     = "Linking shared library ";
        target_name = rest.substr(7);
      }
      else if (rest.starts_with("BIN|"))
      {
        message     = "Linking executable ";
        target_name = rest.substr(4);
      }

      if_.loading_bar(bar_width, percent, message + target_name);
    }
#ifndef DEBUG_MODE
    else if (line.starts_with("make"))
    {
      continue; // do not display make messages
    }
#endif
    else
    {
      if_.clear_loading_bar();
      if_.print(line); // TODO: parse to detect if it is an error / warning to change display style
    }
  }

  if_.clear_loading_bar();

  const int result = pclose(pipe);
  if (WEXITSTATUS(result) == 0) // FIX: find solution for windows
    if_.success("Project was successfully built in " + build_dir.string());
  else
    throw ZCException(
      ZCE_COMPILATION_ERROR, "Build failed with exit code " + to_string(WEXITSTATUS(result))
    );
}

void Project::clean(bool cache) const
{
  if (fs::exists(build_dir) && fs::is_directory(build_dir))
    if (fs::remove_all(build_dir) > 0)
      if_.info("Cleaned " + pretty_path(build_dir));

  if (cache && fs::exists(cache_dir_) && fs::is_directory(cache_dir_))
    if (fs::remove_all(cache_dir_) > 0)
      if_.info("Cleaned " + pretty_path(cache_dir_));
}

void Project::publish()
{
  if_.info("Preparing to publish package " + pconf.name + " at version " + pconf.version.string() + "...");

  for (const auto &dep : pconf.dependencies | views::values)
    if (reg_.get_pkg(dep.name).origin == "local")
      throw ZCException(
        ZCE_LOCAL_DEPENDENCY, "Cannot publish package depending on locally installed package: " + dep.name
      );

  // Version is already in its constructor
  if (pconf.author.empty())
    throw ZCException(ZCE_MISSING_PROPERTY, "Package author is missing in zc.json");
  if (pconf.name.empty())
    throw ZCException(ZCE_MISSING_PROPERTY, "Package name is missing in zc.json");

  if (gc_.token.empty())
    throw ZCException(
      ZCE_MISSING_PROPERTY, "Authentication error: token is empty. Please run 'zc login' first."
    );

  if_.info("Verifying GitHub identity...");
  Network &net(Network::get());
  string   user_info_raw = net.get("https://api.github.com/user", "", gc_.token); // empty payload
  auto     user_info     = nlohmann::json::parse(user_info_raw);
  string   github_login  = user_info["login"];

  if (github_login != pconf.author)
    throw ZCException(
      ZCE_AUTHENTICATION_ERROR, "Identity mismatch: You are logged in as '" + github_login +
                                  "' but the author in zc.json is '" + pconf.author +
                                  "'. Publication blocked for security reasons."
    );

  if_.success("Authenticated as " + github_login);

  string tag = "v" + pconf.version.string();
  string archive_url =
    "https://github.com/" + pconf.author + "/" + pconf.name + "/archive/refs/tags/" + tag + ".tar.gz";

  if_.info("Expected archive URL: " + archive_url);

  const fs::path tmp_dir      = zc_root() / TMP_DIR;
  const fs::path archive_path = tmp_dir / (pconf.name + "_" + tag + ".tar.gz");
  fs::create_directories(tmp_dir);

  // Ensure the release exists on GitHub
  string release_api_url =
    "https://api.github.com/repos/" + pconf.author + "/" + pconf.name + "/releases/tags/" + tag;

  try
  {
    if_.info("Checking if release '" + tag + "' exists on GitHub...");
    net.get(release_api_url, "", gc_.token);
    if_.success("Release " + tag + " found!");
  }
  catch (const ZCException &)
  {
    if_.warning("Release " + tag + " not found. Attempting to create it...");

    if (!if_.ask("Do you want to create the tag " + tag + "?"))
      return;

    nlohmann::json release_payload;
    release_payload["tag_name"]               = tag;
    release_payload["name"]                   = "Release " + tag;
    release_payload["body"]                   = "Automated release by ZC build tool.";
    release_payload["generate_release_notes"] = true;

    string create_release_url =
      "https://api.github.com/repos/" + pconf.author + "/" + pconf.name + "/releases";

    try
    {
      net.post(create_release_url, release_payload.dump(), gc_.token);
      if_.success("Release " + tag + " created successfully!");
    }
    catch (const ZCException &) // TODO: handle different error types directly in Network.cpp
    {
      throw ZCException(
        ZCE_NETWORK_ERROR, "Failed to create release. Ensure the tag exists or you have enough permissions."
      );
    }
  }

  // Download and calculate hash
  if_.info("Downloading archive for verification...");
  net.download(archive_url, archive_path);

  string sha = sha256(archive_path);
  if_.success("SHA-256 calculated: " + sha);
  fs::remove(archive_path);

  // Upload recipe to registry
  json recipe;
  recipe["name"]    = pconf.name;
  recipe["version"] = pconf.version.string();
  recipe["url"]     = archive_url;
  recipe["sha256"]  = sha;
  recipe["owner"]   = pconf.author;

  string file_path = "packages/" + pconf.name + "/" + pconf.version.string() + ".json";
  json   payload;
  payload["message"] = "Publish " + pconf.name + " v" + pconf.version.string();
  payload["content"] = base64_encode(recipe.dump(2));

  string api_url = "https://api.github.com/repos/" GH_REPO "/contents/" + file_path;

  if_.info("Uploading recipe to registry...");
  try
  {
    // Safety Check: Ensure the package owner doesn't change
    string package_dir_url = "https://api.github.com/repos/" GH_REPO "/contents/packages/" + pconf.name;
    try
    {
      if_.debug("Verifying package ownership in registry...");
      string dir_content_raw = net.get(package_dir_url, "", gc_.token);

      if (auto dir_content = nlohmann::json::parse(dir_content_raw);
          dir_content.is_array() && !dir_content.empty())
      {
        // Get the first file found to check the original owner
        string first_version_url = dir_content[0]["download_url"];
        string first_version_raw = net.get(first_version_url, "", gc_.token);

        if (auto first_version_json = nlohmann::json::parse(first_version_raw);
            first_version_json.contains("owner") && first_version_json["owner"] != pconf.author)
        {
          throw ZCException(
            ZCE_AUTHENTICATION_ERROR, "Security Violation: This package is owned by '" +
                                        first_version_json["owner"].get<string>() +
                                        "'. You cannot publish a new version."
          );
        }
      }
    }
    catch (const ZCException &e)
    {
      // 404 is fine, it means it's a new package
      if (string(e.what()).find("404") == string::npos)
        throw;
    }

    // Upload the recipe
    net.put(api_url, payload.dump(), gc_.token);
    if_.success("Your package has been successfully published to the registry!");
  }
  catch (const ZCException &e)
  {
    if (string(e.what()).find("422") != string::npos)
    {
      throw ZCException(
        ZCE_VERSION_ALREADY_EXISTS, "Version " + pconf.version.string() + " already exists in the registry."
      );
    }
    throw ZCException(ZCE_NETWORK_ERROR, "Failed to upload recipe: " + string(e.what()));
  }
}

void Project::add_dependency(const Target &target, const bool is_static)
{
  if (pconf.name == target.name)
    throw ZCException(ZCE_RECURSIVE_DEPENDENCY, "Cannot add package as its own dependency.");

  Dependency dep  = reg_.get_dependency(target); // throws an error if package is not found
  dep.static_link = is_static;

  pconf.add_dependency(dep);
  fs::create_directories(build_dir);
  generate_compile_commands();
}

void Project::remove_dependency(const string &name)
{
  pconf.remove_dependency(name);
  fs::create_directories(build_dir);
  generate_compile_commands();
}

void Project::change_dependency_version(const std::string &name, Version &new_version)
{
  if (new_version.empty())
    new_version = reg_.get_latest(name);
  if (!reg_.is_installed({ name, new_version }))
    throw ZCException(ZCE_PKG_NOT_FOUND, "Package '" + name + "' is not installed");
  pconf.change_dependency_version(name, new_version);
  fs::create_directories(build_dir);
  generate_compile_commands();
}

void Project::install_dependencies() const
{
  if_.info("Installing package dependencies...");
  const auto &net   = Network::get();
  const json  index = net.get_index();
  for (const auto &dep : pconf.dependencies | views::values)
  {
    if (dep.origin == "local")
    {
      if_.warning("Dependency '" + dep.name + "' is a local package. Make sure it's installed.");
      continue;
    }
    if (dep.origin == "std")
    {
      if_.warning(
        "Dependency '" + dep.name + "' is a standard package. Make sure it's installed on your system."
      );
      continue;
    }

    Target t{ .name = dep.name, .version = dep.version };
    reg_.install_from_server(t, index);
  }
}

void Project::update_dependencies()
{
  if_.info("Updating package dependencies...");
  const auto &net   = Network::get();
  const json  index = net.get_index();
  for (const auto &dep : pconf.dependencies | views::values)
  {
    if (dep.origin == "local" || dep.origin == "std")
      continue;

    Target t{ .name = dep.name, .version = Version::latest() };
    reg_.update_from_server(t, index);
    pconf.change_dependency_version(dep.name, Version::latest());
  }
}

void Project::generate_build_config()
{
  if (pconf.type == PkgType::HEADER)
    return;
  fs::create_directories(build_dir);

  bool is_release = false;
  if (const fs::path build_mode_file = build_dir / BUILD_MODE_FILE;
      fs::exists(build_mode_file) && read_file(build_mode_file) == "release")
    is_release = true;

  get_sources();
  init_variables(is_release);
  generate_Makefile();
  generate_compile_commands();
}

void Project::generate_Makefile() const
{
  std::ostringstream mk;

  Makefile_comment(mk);
  Makefile_variables(mk);

  switch (pconf.type)
  {
  case PkgType::BIN:
    Makefile_bin(mk);
    break;
  case PkgType::LIB:
    Makefile_lib(mk);
    break;
  case PkgType::COMPOSE:
  default:
    Makefile_compose(mk);
    break;
  }

  Makefile_rules(mk);

  std::ofstream out(makefile_);
  out << mk.str();
}

void Project::Makefile_bin(std::ostringstream &mk) const
{
  mk << "TARGET := " << BIN_NAME(pconf.target) << "\n\n";
  mk << "all: $(TARGET)\n\n";

  mk << "$(TARGET):";
  for (const auto &l : pconf.languages | views::keys)
    mk << " $(" << language_to_str(l) << "_OBJS)";
  mk << "\n";
  mk << "\t@echo \"ZC_BIN|$@\"\n";
  mk << "\t@" << get_linker() << " -o $@ $^ $(LIB_DIRS) $(LIBS)\n\n";
}

void Project::Makefile_lib(std::ostringstream &mk) const
{
  mk << "TARGET_STATIC := " << STATIC_LIB_NAME(pconf.target) << "\n";
  mk << "TARGET_SHARED := " << SHARED_LIB_NAME(pconf.target) << "\n\n";
  mk << "all: $(TARGET_STATIC) $(TARGET_SHARED)\n\n";

  mk << "$(TARGET_STATIC):";
  for (const auto &l : pconf.languages | views::keys)
    mk << " $(" << language_to_str(l) << "_OBJS)";
  mk << "\n";
  mk << "\t@echo \"ZC_STATIC|$@\"\n";
  mk << "\t@" << gc_.archive << " $@ $^\n\n";

  mk << "$(TARGET_SHARED):";
  for (const auto &l : pconf.languages | views::keys)
    mk << " $(" << language_to_str(l) << "_OBJS)";
  mk << "\n";
  mk << "\t@echo \"ZC_SHARED|$@\"\n";
  mk << "\t@" << get_linker() << " -shared -o $@ $^ $(LIB_DIRS) $(LIBS)\n\n";
}

void Project::Makefile_compose(std::ostringstream &mk)
{
  mk << "all:\n";
  mk << "\t@echo \"Compose project type is not yet fully implemented\"\n\n";
}

void Project::generate_compile_commands() const
{
  json compile_commands = nlohmann::json::array();

  string includes;
  if (auto it = variables_.find("INCLUDE_DIRS"); it != variables_.end())
    includes = it->string();

  string macros;
  if (auto it = variables_.find("MACROS"); it != variables_.end())
    macros = it->string();

  for (const auto &l : pconf.languages)
  {
    if (!sources_.contains(l.first))
      continue;

    string flags;
    if (auto it = variables_.find(language_to_str(l.first) + "_FLAGS"); it != variables_.end())
      flags = it->string();

    for (const auto &file : sources_.at(l.first))
    {
      nlohmann::json cmd;
      cmd["directory"] = fs::absolute(build_dir).string();
      cmd["file"]      = "../" + file;
      cmd["output"]    = file + ".o";
      cmd["command"] =
        join({ l.second.compiler, flags, includes, macros, "-c", "../" + file, "-o", file + ".o" });
      compile_commands.push_back(cmd);
    }
  }

  std::ofstream out(build_dir / "compile_commands.json");
  out << compile_commands.dump(2);
}

void Project::Makefile_comment(std::ostringstream &mk)
{
  const string s = std::format("{:%F %T}", chrono::floor<chrono::seconds>(chrono::system_clock::now()));
  mk << "# --- This file was automatically generated by ZC\n";
  mk << "# --- Date of creation: " << s << " (UTC)\n";
  mk << "# --- !! Do not edit this file manually !!\n\n";

#ifdef DEBUG_MODE // Make boilerplate
  mk << "# Remove implicit rules to analyse the Makefile faster\n";
#endif
  mk << "MAKEFLAGS += --no-builtin-rules\n";
  mk << "MAKEFLAGS += --no-builtin-variables\n";
#ifdef DEBUG_MODE
  mk << "# Warn when undefined variables are used\n";
  mk << "MAKEFLAGS += --warn-undefined-variables\n\n";
#endif

#ifdef DEBUG_MODE
  mk << "# Delete target when a compiling error occurred\n";
#endif
  mk << ".DELETE_ON_ERROR:\n\n";
#ifdef DEBUG_MODE
  mk << "# Path to binaries\n";
#endif
  mk << "VPATH = ..\n\n";
  mk << ".PHONY: all clean install\n\n";

  mk << "all:\n\n"; // Prevent -include from hijacking the default target by explicitly declaring all first
}

void Project::Makefile_variables(ostringstream &mk) const
{
  for (const auto &v : variables_)
    mk << v.make_declaration();
  for (const auto &l : pconf.languages | views::keys)
    mk << "-include $(" << language_to_str(l) << "_DEPS)\n\n";
}

void Project::Makefile_rules(std::ostringstream &mk) const
{
  mk << "clean:\n\tzc clean\n\n"; // FIX : will crash on Windows (cannot delete build/ where make operates)
  mk << "install:\n\tzc install --path ..\n\n";
  mk << "help:\n";
  mk << "\t@echo \"Available targets:\"\n";
  mk << "\t@echo \"  all      - Build the project\"\n";
  mk << "\t@echo \"  clean    - Clean the project\"\n";
  mk << "\t@echo \"  install  - Install the project\"\n";
  mk << "\t@echo \"  help     - Show this help message\"\n\n";

  for (const auto &l : pconf.languages | views::keys)
  {
    const string name = language_to_str(l);
    for (const auto &ext : extensions_for_language(l))
    {
      const string lower_ext = lower(ext);
      mk << "%." << lower_ext << ".o: %." << lower_ext << " ../" << ZC_FILE << "\n";
      mk << "\t@" MKDIR_COMMAND " $(dir $@)\n";
      mk << "\t@echo \"ZC_COMPILE|$@\"\n";
      mk << "\t@$(" << name << "_COMPILER) $(" << name
         << "_FLAGS) $(INCLUDE_DIRS) $(MACROS) -c $< -o $@\n\n";

      mk << "%." << ext << ".o: %." << ext << " ../" << ZC_FILE << "\n";
      mk << "\t@" MKDIR_COMMAND " $(dir $@)\n";
      mk << "\t@echo \"ZC_COMPILE|$@\"\n";
      mk << "\t@$(" << name << "_COMPILER) $(" << name
         << "_FLAGS) $(INCLUDE_DIRS) $(MACROS) -c $< -o $@\n\n";
    }
  }
}

int Project::get_sources()
{
  // Check if include directories exist
  for (const auto &inc : pconf.include_dirs)
    if (const fs::path full_inc_dir = root_dir / inc; !fs::exists(full_inc_dir))
      throw ZCException(ZCE_NOT_FOUND, "Include dir does not exist: " + full_inc_dir.string());

  sources_.clear(); // security
  int total_files_to_compile = 0;
  for (const auto &src_dir : pconf.src_dirs)
  {
    const fs::path full_src_dir = root_dir / src_dir;
    if (!fs::exists(full_src_dir))
      throw ZCException(ZCE_NOT_FOUND, "Source dir " + full_src_dir.string() + " not found.");

    for (const auto &entry : fs::recursive_directory_iterator(full_src_dir))
    {
      const fs::path &file = entry.path();
      if (const Language l = language_of(file); l != UNKNOWN_LANGUAGE)
      {
        if (sources_.contains(l))
          sources_[l].push_back(fs::relative(file, root_dir).string());
        else
          sources_[l] = { fs::relative(file, root_dir).string() };
        total_files_to_compile++;
      }
    }
  }

  if (total_files_to_compile == 0)
    throw ZCException(ZCE_NO_SOURCE_FILES, "No source files were found.");

  return total_files_to_compile;
}

std::string Project::get_linker() const
{
  if (sources_.contains(CXX))
    return "$(CXX_COMPILER)";
  return "$(C_COMPILER)";
}

void Project::init_variables(bool release)
{
  for (const auto &l : pconf.languages)
  {
    if (!sources_.contains(l.first))
      throw ZCException(
        ZCE_NO_SOURCE_FILES,
        "Language " + language_to_str(l.first) + " is given but no source files of this language were found"
      );

    // Languages configuration and flags
    string name = language_to_str(l.first);

    MakeVariable compiler{ name + "_COMPILER" };
    compiler.add(l.second.compiler);
    variables_.insert(compiler);

    MakeVariable flags{ name + "_FLAGS" };
    flags.add("-std=" + l.second.std);
    flags.add("-MMD");
    flags.add("-MP");
    flags.add("-fdiagnostics-color=always");
    if (pconf.type == PkgType::LIB)
      flags.add("-fPIC");
    if (release)
      flags.add("-O3");
    else
      flags.add("-g");
    for (const auto &flag : l.second.flags)
      flags.add(flag);
    variables_.insert(flags);

    MakeVariable objs{ name + "_OBJS" };
    for (const auto &file : sources_.at(l.first))
      objs.add_no_esc(file + ".o");
    variables_.insert(objs);

    MakeVariable deps{ name + "_DEPS" };
    deps.add_make_var(name + "_OBJS:.o=.d");
    variables_.insert(deps);
  }

  const fs::path cache_dir = zc_root() / ZC_CACHE_DIR;

  // Macros
  MakeVariable macros{ "MACROS" };
  macros.add(release ? "-DZC_RELEASE" : "-DZC_DEBUG");
  macros.add("-DZC_MAJOR=" + to_string(pconf.version.major()));
  macros.add("-DZC_MINOR=" + to_string(pconf.version.minor()));
  macros.add("-DZC_PATCH=" + to_string(pconf.version.patch()));
  macros.add("-DZC_VERSION=\"" + pconf.version.string() + "\"");

  // Libraries and include directories
  MakeVariable incdirs{ "INCLUDE_DIRS" };
  MakeVariable libdirs{ "LIB_DIRS" };
  MakeVariable libs{ "LIBS" };
  for (const auto &inc : pconf.include_dirs)
    incdirs.add("-I../" + inc);
  for (const auto &[name, origin, static_link, version] : pconf.dependencies | views::values)
  {
    if (origin == "std")
    {
      for (const auto flags = split(get_pkg_config_flags(name, true), ' '); const auto &f : flags)
      {
        if (f.starts_with("-I"))
          incdirs.add(f);
        elif (f.starts_with("-l"))
          libs.add(f);
        elif (f.starts_with("-L"))
          libdirs.add(f);
        else
          macros.add(f);
      }
      continue;
    }
    const fs::path pkg_dir = cache_dir / name / version.string();
    const fs::path inc_dir = pkg_dir / INCLUDE_DIR;
    incdirs.add("-I" + inc_dir.string());

    Pkg pkg = reg_.get_pkg(name);
    if (pkg.type == PkgType::HEADER)
      continue;

    const fs::path lib_dir = pkg_dir / LIB_DIR;
    libdirs.add("-L" + lib_dir.string());

    if (static_link) // We add the archive directly as a source
      libs.add((lib_dir / STATIC_LIB_NAME(pkg.target)).string());
    else
    {
      libs.add("-l" + pkg.target);
      libdirs.add("-Wl,-rpath," + lib_dir.string());
    }
  }
  variables_.insert(macros);
  variables_.insert(incdirs);
  variables_.insert(libdirs);
  variables_.insert(libs);
}

} // namespace zc
