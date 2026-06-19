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
#include "pkgs/Registry.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

/**
 * Project implementation
 */

Project::Project(const std::filesystem::path &root)
    : root_dir(root), build_dir(root / BUILD_DIR), cache_dir_(root / PROJECT_CACHE_DIR),
      makefile_(build_dir / MAKEFILE)
{
}

void Project::build(const bool release)
{
  if_.info("Building package " + pconf.name + "...");
  const string make_cmd = "make -C " + build_dir.string();
  int to_compile = 0;
  int compiled = 0;

  if (release)
  {
    install_dependencies();
    to_compile = generate_Makefile(release);
  }
  else
  {
    generate_compile_commands();
    if (!fs::exists(makefile_))
      to_compile = generate_Makefile();
    else
    {
      const string dry_run_cmd = make_cmd + " -n 2>/dev/null";
      FILE *dry_pipe = popen(dry_run_cmd.c_str(), "r");
      if (!dry_pipe)
        throw ZCException(ZCE_INTERNAL_ERROR, "Failed to run make");

      char buffer[512];
      while (fgets(buffer, sizeof(buffer), dry_pipe) != nullptr)
      {
        if (string(buffer).find("ZC_COMPILE|") != string::npos)
          to_compile++;
      }
      pclose(dry_pipe);
    }
  }

  if (to_compile == 0)
  {
    if_.success("Project is already up to date! Nothing to do.");
    return;
  }

  FILE *pipe = popen((make_cmd + " 2>&1").c_str(), "r");
  if (!pipe)
    throw ZCException(ZCE_INTERNAL_ERROR, "Failed to run make");

  char buffer[1024];

  while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
  {
    string line(buffer);
    if (line.starts_with("ZC_COMPILE|"))
    {
      compiled++;
      int percent = (compiled * 100) / to_compile;
      if (percent > 100)
        percent = 100;

      // Extract filename
      string filename = line.substr(11); // After ZC_COMPILE|
      if (!filename.empty() && filename.back() == '\n')
        filename.pop_back();

      const int bar_width = 20;
      if_.loading_bar(bar_width, percent, "Compiling " + filename);
    }
    else
    {
      // Pass other outputs (warnings, errors) directly, but clear the progress bar line first
      if_.clear_loading_bar();
      if_.info(line); // TODO : parse to detect if it is an error / warning to change display style
    }
  }

  if_.new_line();

  const int result = pclose(pipe);
  if (WEXITSTATUS(result) == 0)
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

  // Version is already in its constructor
  if (pconf.author.empty())
    throw ZCException(ZCE_MISSING_PROPERTY, "Package author is missing in zc.json");
  if (pconf.name.empty())
    throw ZCException(ZCE_MISSING_PROPERTY, "Package name is missing in zc.json");

  GConf &gc(GConf::get());
  if (gc.token.empty())
    throw ZCException(
        ZCE_MISSING_PROPERTY, "Authentication error: token is empty. Please run 'zc login' first."
    );

  if_.info("Verifying GitHub identity...");
  Network &net(Network::get());
  string user_info_raw = net.get("https://api.github.com/user", "", gc.token); // empty payload
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
    net.get(release_api_url, "", gc.token);
    if_.success("Release " + tag + " found!");
  }
  catch (const ZCException &)
  {
    if_.warning("Release " + tag + " not found. Attempting to create it...");

    if (!if_.ask("Do you want to create tag " + tag + "?"))
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
      net.post(create_release_url, release_payload.dump(), gc.token);
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
      string dir_content_raw = net.get(package_dir_url, "", gc.token);

      if (auto dir_content = nlohmann::json::parse(dir_content_raw);
          dir_content.is_array() && !dir_content.empty())
      {
        // Get the first file found to check the original owner
        string first_version_url = dir_content[0]["download_url"];
        string first_version_raw = net.get(first_version_url, "", gc.token);

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
    net.put(api_url, payload.dump(), gc.token);
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

void Project::add_dependency(const string &name)
{
  RegistryPkg pkg = reg_.get_pkg(name); // throws an error if package is not found

  const Dependency d{.name = pkg.name, .static_link = false, .version = *ranges::max_element(pkg.versions)};

  pconf.add_dependency(d);
  // TODO : regenerate Makefile ?
  generate_compile_commands(); // for the LSPs
}

void Project::remove_dependency(const string &name)
{
  pconf.remove_dependency(name);
  generate_compile_commands(); // for the LSPs
}

void Project::install_dependencies() const
{
  const auto &net = Network::get();
  const json index = net.get_index();

  for (const auto &dep : pconf.dependencies) reg_.install_from_server(dep.name, dep.version.string(), index);
}

void Project::update_dependencies() const
{
  const auto &net = Network::get();
  const json index = net.get_index();
  // "" = get latest version
  for (const auto &dep : pconf.dependencies) reg_.update_from_server(dep.name, "", index);
}

void Project::generate_build_config()
{
  generate_Makefile();
  generate_compile_commands();
}

int Project::generate_Makefile(const bool release)
{
  if (pconf.type == HEADER)
    return 0;

  GConf &gc(GConf::get());
  fs::create_directories(build_dir);

  const int to_compile = get_sources(); // also checks if include dirs exist

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

  return to_compile;
}

void Project::Makefile_bin(std::ostringstream &mk) const
{
  mk << "all: $(TARGET)\n\n";
  mk << "$(TARGET):";
  for (const auto &l : pconf.languages) mk << " $(" << language_to_str(l.name) << "_OBJS)";
  mk << "\n";
  mk << "\t@echo \"ZC_LINK $@\"\n";
  mk << "\t@$(CXX) $(CXXFLAGS) -o $@ $^\n\n";
}

void Project::Makefile_lib(std::ostringstream &mk) const
{
  mk << "$(TARGET): $(OBJS)\n";
  mk << "\t@echo \"[LINK] $@\"";
  mk << "\t$()\n";

  mk << "TARGET_STATIC = " << STATIC_LIB_NAME << "\n"; // TODO : handle other operating systems
  mk << "TARGET_SHARED = " << SHARED_LIB_NAME << "\n\n";
  mk << "all: $(TARGET_STATIC) $(TARGET_SHARED)\n\n";

  mk << "$(TARGET_STATIC): $(COBJS) $(CXXOBJS)\n";
  mk << "\t@echo \"[AR]   Archiving static library $@\"\n";
  mk << "\t@ar rcs $@ $^\n\n";

  mk << "$(TARGET_SHARED): $(COBJS) $(CXXOBJS)\n";
  mk << "\t@echo \"[Link] Linking shared library $@\"\n";
  mk << "\t@$(CXX) -shared -o $@ $^\n\n";
}

void Project::Makefile_compose(std::ostringstream &mk) const
{
  mk << "all:\n";
  mk << "\t@echo \"Compose project type is not yet fully implemented\"\n\n";
}

void Project::generate_compile_commands() const
{
  fs::create_directories(build_dir);
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
  mk << ".PHONY: all clean install";

  for (const auto &l : pconf.languages)
  {
    // Languages configuration and flags
    string name = language_to_str(l.name);
    mk << name << "_COMPILER := " << l.compiler << "\n";
    mk << name << "_STD := " << l.std << "\n";
    mk << name << "_FLAGS := -std=${" << name << "_STD} -MMD -MP";
    for (const auto &flag : l.flags) mk << " " << escape_shell_arg(flag);

    if (pconf.type == LIB)
      mk << " -fPIC";
    if (release)
      mk << " -O3 -DZC_RELEASE";
    else
      mk << " -g -DZC_DEBUG";
    mk << "\n";

    // Files to compile into objects
    mk << name << "_OBJS :=";
    for (const auto &file : sources_.at(l.name)) mk << " " << file << ".o";
    mk << "\n";

    // Dependencies files
    mk << name << "_DEPS := $(" << name << "_OBJS:.o=.d)\n";
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
  mk << "LIB_DIRS :=";
  for (const auto &dep : pconf.dependencies) // TODO : handle std libraries
    mk << " -L" << (cache_dir / dep.name / dep.version.string() / LIB_DIR).string();
  mk << "\n";

  // Libraries for linker
  mk << "LIBS :=";
  for (const auto &dep : pconf.dependencies) mk << " -l" << reg_.get_pkg(dep.name).target;
  mk << "\n";

  // Target
  mk << "TARGET := " << pconf.target;
  mk << "\n";
}

void Project::Makefile_rules(std::ostringstream &mk) const
{
  mk << "clean:\n\tzc clean\n\n";
  mk << "install:\n\tzc install --path ..\n\n";

  for (const auto &l : pconf.languages)
  {
    const string name = language_to_str(l.name);
    for (const auto &ext : extensions_for_language(l.name))
    {
      mk << "%." << ext << ".o: ../%";
      mk << "\t" MKDIR_COMMAND " $(dir $@)\n";
      mk << "\t@echo \"ZC_COMPILE|$<\"\n";
      mk << "\t@$(" << name << "_COMPILER) $(" << name << "_FLAGS) $(INCLUDE_DIRS) -c $< -o $@\n\n";

      const string lower_ext = lower(ext);
      mk << "%." << lower_ext << ".o: ../%";
      mk << "\t" MKDIR_COMMAND " $(dir $@)\n";
      mk << "\t@echo \"ZC_COMPILE|$<\"\n";
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
          sources_[l].push_back(file.string());
        else
          sources_[l] = {file.string()};
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
  return "g++"; // TODO : implement
}

} // namespace zc
