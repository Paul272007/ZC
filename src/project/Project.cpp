/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

#include "../config/GConf.h"
#include "../helpers.h"
#include "Language.h"
#include "Project.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "pkgs/Network.h"
#include "pkgs/PkgType.h"
#include "pkgs/Registry.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

Project::Project(const std::filesystem::path &root)
    : root_dir(root), build_dir(root / BUILD_DIR), pconf(root / ZC_FILE),
      cache_dir_(root / PROJECT_CACHE_DIR), makefile_(build_dir / MAKEFILE)
{
}

void Project::build(BuildMode current_mode)
{
  if (pconf.type == HEADER)
    return;

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
    }
  }
  else // if build mode file doesn't exist and mode is set to automatic, default is debug
  {
    current_mode = BuildMode::debug;
  }

  fs::create_directories(build_dir);

  write_file(build_mode_file, build_mode_to_str(current_mode));

  const string make_cmd = "make --no-print-directory -C " + build_dir.string();
  int compiled = 0;

  // to_compile_ = get_sources(); // TODO : don't count sources to compile in this function
  get_sources();

  if (current_mode == BuildMode::debug)
    generate_compile_commands();

  generate_Makefile(current_mode == BuildMode::release);

  to_compile_ = 0;
  const string dry_run_cmd = make_cmd + " -n 2>/dev/null";
  FILE *dry_pipe = popen(dry_run_cmd.c_str(), "r");
  if (!dry_pipe)
    throw ZCException(ZCE_INTERNAL_ERROR, "Failed to run make");

  char buffer_dry[512];
  while (fgets(buffer_dry, sizeof(buffer_dry), dry_pipe) != nullptr)
    if (string(buffer_dry).find("ZC_COMPILE|") != string::npos)
      to_compile_++;
  pclose(dry_pipe);

  if (to_compile_ == 0)
  {
    if_.success("Project is already up to date! Nothing to do.");
    return;
  }
  if_.info(std::to_string(to_compile_) + " file(s) to compile");

  // Add linking as compilation step
  to_compile_ += pconf.type == BIN ? 1 : (pconf.type == LIB ? 2 : 0);

  FILE *pipe = popen((make_cmd + " 2>&1").c_str(), "r");
  if (!pipe)
    throw ZCException(ZCE_INTERNAL_ERROR, "Failed to run make");

  char buffer[1024];
  const int bar_width = 20;

  while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
  {
    string line(buffer);
    if (!line.empty() && line.back() == '\n')
      line.pop_back();

    if (line.starts_with("ZC_"))
    {
      compiled++;
      int percent = (compiled * 100) / to_compile_;
      if (percent > 100)
        percent = 100;

      string message;
      string target_name;
      string rest = line.substr(3);

      if (rest.starts_with("COMPILE|"))
      {
        message = "Compiling object ";
        target_name = rest.substr(8);
      }
      else if (rest.starts_with("STATIC|"))
      {
        message = "Linking static library ";
        target_name = rest.substr(7);
      }
      else if (rest.starts_with("SHARED|"))
      {
        message = "Linking shared library ";
        target_name = rest.substr(7);
      }
      else if (rest.starts_with("BIN|"))
      {
        message = "Linking executable ";
        target_name = rest.substr(4);
      }

      if_.loading_bar(bar_width, percent, message + target_name);
    }
    else
    {
      if_.clear_loading_bar();
      if_.print(line); // TODO : parse to detect if it is an error / warning to change display style
    }
  }

  if_.clear_loading_bar();
  // if_.new_line();

  const int result = pclose(pipe);
  if (WEXITSTATUS(result) == 0) // FIX : find solution for windows
    if_.success("Project was successfully built in " + build_dir.string());
  else
    throw ZCException(ZCE_COMPILATION_ERROR, "Build failed with exit code " + to_string(WEXITSTATUS(result)));
}

void Project::clean() const
{
  if (fs::exists(build_dir) && fs::is_directory(build_dir))
    if (fs::remove_all(build_dir) > 0)
      if_.info("Cleaned " + build_dir.string());

  if (fs::exists(cache_dir_) && fs::is_directory(cache_dir_))
    if (fs::remove_all(cache_dir_) > 0)
      if_.info("Cleaned " + cache_dir_.string());
}

void Project::publish()
{
  if_.info("Preparing to publish package " + pconf.name + " at version " + pconf.version.string() + "...");

  for (const auto &dep : pconf.dependencies)
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
  string user_info_raw = net.get("https://api.github.com/user", "", gc_.token); // empty payload
  auto user_info = nlohmann::json::parse(user_info_raw);
  string github_login = user_info["login"];

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

  const fs::path tmp_dir = get_zc_root() / TMP_DIR;
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
    release_payload["tag_name"] = tag;
    release_payload["name"] = "Release " + tag;
    release_payload["body"] = "Automated release by ZC build tool.";
    release_payload["generate_release_notes"] = true;

    string create_release_url =
        "https://api.github.com/repos/" + pconf.author + "/" + pconf.name + "/releases";

    try
    {
      net.post(create_release_url, release_payload.dump(), gc_.token);
      if_.success("Release " + tag + " created successfully!");
    }
    catch (const ZCException &) // TODO : handle different error types directly in Network.cpp
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
  recipe["name"] = pconf.name;
  recipe["version"] = pconf.version.string();
  recipe["url"] = archive_url;
  recipe["sha256"] = sha;
  recipe["owner"] = pconf.author;

  string file_path = "packages/" + pconf.name + "/" + pconf.version.string() + ".json";
  json payload;
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

void Project::add_dependency(const string &name, const bool is_static)
{
  if (pconf.name == name)
    throw ZCException(ZCE_RECURSIVE_DEPENDENCY, "Cannot add package as its own dependency.");

  RegistryPkg pkg = reg_.get_pkg(name); // throws an error if package is not found
  if (pkg.type == BIN)
    throw ZCException(ZCE_TYPE_ERROR, "Cannot add dependency of type BIN as a dependency");

  const Dependency d{
      .name = pkg.name,
      .origin = pkg.origin,
      .static_link = is_static,
      .version = *ranges::max_element(pkg.versions)
  };

  pconf.add_dependency(d);
  fs::create_directories(build_dir);
  generate_compile_commands();
}

void Project::remove_dependency(const string &name)
{
  pconf.remove_dependency(name);
  fs::create_directories(build_dir);
  generate_compile_commands();
}

void Project::change_dependency_version(const std::string &name, const Version &new_version)
{
  if (!reg_.is_installed({name, new_version}))
    throw ZCException(ZCE_PKG_NOT_FOUND, "Package '" + name + "' was not found");

  pconf.change_dependency_version(name, new_version);
}

void Project::install_dependencies() const
{
  if_.info("Installing package dependencies...");
  const auto &net = Network::get();
  const json index = net.get_index();
  for (const auto &dep : pconf.dependencies)
  {
    if (dep.origin == "local")
    {
      if_.warning(
          "Dependency '" + dep.name +
          "' is a local package. Skipping server installation. Make sure it's installed locally."
      );
      continue;
    }
    if (dep.origin == "std")
    {
      if_.warning(
          "Dependency '" + dep.name +
          "' is a standard system package. Make sure it's installed on your system."
      );
      continue;
    }

    Target t{.name = dep.name, .version = dep.version};
    reg_.install_from_server(t, index);
  }
}

void Project::update_dependencies() const
{
  if_.info("Updating package dependencies...");
  const auto &net = Network::get();
  const json index = net.get_index();
  for (const auto &dep : pconf.dependencies)
  {
    if (dep.origin == "local" || dep.origin == "std")
      continue;

    Target t{.name = dep.name, .version = dep.version};
    reg_.update_from_server(t, index);
  }
}

void Project::generate_build_config()
{
  if (pconf.type == HEADER)
    return;
  fs::create_directories(build_dir);

  bool is_release = false;
  if (const fs::path build_mode_file = build_dir / BUILD_MODE_FILE;
      fs::exists(build_mode_file) && read_file(build_mode_file) == "release")
    is_release = true;

  get_sources();
  generate_Makefile(is_release);
  generate_compile_commands();
}

void Project::generate_Makefile(const bool release) const
{
  std::ostringstream mk;

  Makefile_comment(mk);
  Makefile_variables(mk, release);

  switch (pconf.type)
  {
  case BIN:
    Makefile_bin(mk);
    break;
  case LIB:
    Makefile_lib(mk);
    break;
  case COMPOSE:
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
  for (const auto &l : pconf.languages) mk << " $(" << language_to_str(l.name) << "_OBJS)";
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
  for (const auto &l : pconf.languages) mk << " $(" << language_to_str(l.name) << "_OBJS)";
  mk << "\n";
  mk << "\t@echo \"ZC_STATIC|$@\"\n";
  mk << "\t@" << gc_.archive << " $@ $^\n\n";

  mk << "$(TARGET_SHARED):";
  for (const auto &l : pconf.languages) mk << " $(" << language_to_str(l.name) << "_OBJS)";
  mk << "\n";
  mk << "\t@echo \"ZC_SHARED|$@\"\n";
  mk << "\t@" << get_linker() << " -shared -o $@ $^ $(LIB_DIRS) $(LIBS)\n\n";
}

void Project::Makefile_compose(std::ostringstream &mk) const
{
  mk << "all:\n";
  mk << "\t@echo \"Compose project type is not yet fully implemented\"\n\n";
}

void Project::generate_compile_commands() const
{
  nlohmann::json compile_commands = nlohmann::json::array();
  const fs::path cache_dir = get_zc_root() / ZC_CACHE_DIR;

  string includes = "";
  for (const auto &inc : pconf.include_dirs) includes += " -I../" + inc;
  for (const auto &dep : pconf.dependencies)
    includes += " -I" + (cache_dir / dep.name / dep.version.string() / INCLUDE_DIR).string();

  for (const auto &l : pconf.languages)
  {
    if (sources_.find(l.name) == sources_.end())
      continue;

    string flags = "-std=" + l.std + " -MMD -MP";
    for (const auto &flag : l.flags) flags += " " + escape_shell_arg(flag);
    if (pconf.type == LIB)
      flags += " -fPIC";
    flags += " -g -DZC_DEBUG"; // Always generate compile_commands in debug mode by default for LSP

    for (const auto &file : sources_.at(l.name))
    {
      nlohmann::json cmd;
      cmd["directory"] = fs::absolute(build_dir).string();
      cmd["file"] = "../" + file;
      cmd["output"] = file + ".o";
      cmd["command"] = l.compiler + " " + flags + includes + " -c ../" + file + " -o " + file + ".o";
      compile_commands.push_back(cmd);
    }
  }

  std::ofstream out(build_dir / "compile_commands.json");
  out << compile_commands.dump(2);
}

void Project::Makefile_comment(std::ostringstream &mk) const
{
  auto now_sec = chrono::floor<chrono::seconds>(chrono::system_clock::now());
  const string s = std::format("{:%F %T}", now_sec);
  mk << "# --- This file was automatically generated by ZC\n";
  mk << "# --- Date of creation: " << s << " (UTC)\n";
  mk << "# --- Do not edit this file manually !\n\n";
}

void Project::Makefile_variables(ostringstream &mk, const bool release) const
{
  // Make boilerplate
#ifdef DEBUG_MODE
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

  for (const auto &l : pconf.languages)
  {
    if (auto it = sources_.find(l.name); it == sources_.end())
      throw ZCException(
          ZCE_NO_SOURCE_FILES,
          "Language " + language_to_str(l.name) + " is given but no source files of this language were found"
      );

    // Languages configuration and flags
    string name = language_to_str(l.name);
    mk << name << "_COMPILER := " << l.compiler << "\n";
    mk << name << "_STD      := " << l.std << "\n";
    mk << name << "_FLAGS    := -std=$(" << name << "_STD) -MMD -MP -fdiagnostics-color=always";
    for (const auto &flag : l.flags) mk << " " << escape_shell_arg(flag);

    if (pconf.type == LIB)
      mk << " -fPIC";
    if (release)
      mk << " -O3 -DZC_RELEASE";
    else
      mk << " -g -DZC_DEBUG";
    mk << "\n";

    // Files to compile into objects
    mk << name << "_OBJS     :=";
    for (const auto &file : sources_.at(l.name)) mk << " " << file << ".o";
    mk << "\n";

    // Dependencies files
    mk << name << "_DEPS     := $(" << name << "_OBJS:.o=.d)\n\n";
#ifdef DEBUG_MODE
    mk << "# Do not crash if dependencies do not exist yet\n";
#endif
    mk << "-include $(" << name << "_DEPS)\n\n";
  }

  // Include dirs
  const fs::path cache_dir = get_zc_root() / ZC_CACHE_DIR;
  mk << "INCLUDE_DIRS :=";
  for (const auto &inc : pconf.include_dirs) mk << " -I../" << inc;
  for (const auto &dep : pconf.dependencies) // TODO : handle std libraries
    mk << " -I" << (cache_dir / dep.name / dep.version.string() / INCLUDE_DIR).string();
  mk << "\n";

  // Library dirs
  mk << "LIB_DIRS     :=";
  for (const auto &dep : pconf.dependencies) // TODO : handle std libraries
  {
    if (dep.static_link) // We add the archive directly as a source
      continue;
    const string dep_lib_dir = (cache_dir / dep.name / dep.version.string() / LIB_DIR).string();
    mk << " -L" << dep_lib_dir << " -Wl,-rpath," << dep_lib_dir;
  }
  mk << "\n";

  // Libraries for linker
  mk << "LIBS         :=";
  for (const auto &dep : pconf.dependencies)
  {
    const string dep_target = reg_.get_pkg(dep.name).target;
    if (dep.static_link)
      mk << " " << (cache_dir / dep.name / dep.version.string() / LIB_DIR / STATIC_LIB_NAME(dep_target));
    else
      mk << " -l" << dep_target;
  }
  mk << "\n\n";
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

  for (const auto &l : pconf.languages)
  {
    const string name = language_to_str(l.name);
    for (const auto &ext : extensions_for_language(l.name))
    {
      const string lower_ext = lower(ext);
      mk << "%." << lower_ext << ".o: %." << lower_ext << " ../" << ZC_FILE << "\n";
      mk << "\t@" MKDIR_COMMAND " $(dir $@)\n";
      mk << "\t@echo \"ZC_COMPILE|$@\"\n";
      mk << "\t@$(" << name << "_COMPILER) $(" << name << "_FLAGS) $(INCLUDE_DIRS) -c $< -o $@\n\n";

      mk << "%." << ext << ".o: %." << ext << " ../" << ZC_FILE << "\n";
      mk << "\t@" MKDIR_COMMAND " $(dir $@)\n";
      mk << "\t@echo \"ZC_COMPILE|$@\"\n";
      mk << "\t@$(" << name << "_COMPILER) $(" << name << "_FLAGS) $(INCLUDE_DIRS) -c $< -o $@\n\n";
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
      const fs::path file = entry.path();
      if (const Language l = language_of(file); l != UNKNOWN_LANGUAGE)
      {
        if (sources_.find(l) != sources_.end())
          sources_[l].push_back(fs::relative(file, root_dir).string());
        else
          sources_[l] = {fs::relative(file, root_dir).string()};
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
  if (sources_.find(CXX) != sources_.end())
    return "$(CXX_COMPILER)";
  else
    return "$(C_COMPILER)";
}

} // namespace zc
