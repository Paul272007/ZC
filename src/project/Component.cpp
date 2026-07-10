#include "project/Component.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <ranges>
#include <sstream>

#include "config/Dependency.h"
#include "config/GConf.h"
#include "config/Language.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "pkgs/PkgType.h"
#include "pkgs/Registry.h"
#include "pkgs/RemoteTarget.h"
#include "project/MakeVariable.h"
#include "ui/Interface.h"
#include "Version.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

Component::Component(std::filesystem::path root)
  : root_dir(std::move(root)), build_dir(root_dir / BUILD_DIR), cconf(root_dir / COMPONENT_FILE)
{
}

void Component::resolve_dependencies(
  const std::map<std::string, Component> &all_components, std::unordered_set<std::string> &visited
)
{
  // Start with our own properties
  resolved_include_dirs = cconf.include_dirs;
  resolved_required     = cconf.required;
  resolved_macros       = cconf.macros;
  resolved_dependencies = cconf.dependencies;

  // Prefix our own include dirs with component name (relative to project root)
  const std::string comp_name = root_dir.filename().string();
  for (auto &inc : resolved_include_dirs)
    if (!inc.starts_with(comp_name + "/"))
      inc = comp_name + "/" + inc;

  // Recursively merge from required components
  for (const auto &req : cconf.required)
  {
    if (visited.contains(req))
      continue;
    visited.insert(req);

    if (!all_components.contains(req))
      throw ZCException(ZCE_NOT_FOUND, "Required component '" + req + "' does not exist.");

    // The required component must already be resolved (or we resolve it now)
    auto &req_comp = const_cast<Component &>(all_components.at(req));
    if (req_comp.resolved_include_dirs.empty() && !req_comp.cconf.include_dirs.empty())
      req_comp.resolve_dependencies(all_components, visited);

    // Merge include dirs
    for (const auto &inc : req_comp.resolved_include_dirs)
      if (std::ranges::find(resolved_include_dirs, inc) == resolved_include_dirs.end())
        resolved_include_dirs.push_back(inc);

    // Merge macros
    for (const auto &[m_name, m_val] : req_comp.resolved_macros)
      resolved_macros[m_name] = m_val;

    // Merge external dependencies
    for (const auto &[d_name, d_val] : req_comp.resolved_dependencies)
      resolved_dependencies[d_name] = d_val;

    // Merge required list (flattened)
    if (std::ranges::find(resolved_required, req) == resolved_required.end())
      resolved_required.push_back(req);
    for (const auto &sub_req : req_comp.resolved_required)
      if (std::ranges::find(resolved_required, sub_req) == resolved_required.end())
        resolved_required.push_back(sub_req);
  }
}

// ============================================================================
// Build
// ============================================================================

void Component::generate_build_config(
  const std::map<Language, LanguageConf> &languages,
  const Version                          &project_version,
  const std::filesystem::path            &project_root,
  bool release, bool is_install
)
{
  if (cconf.type == PkgType::HEADER)
    return;

  fs::create_directories(build_dir);

  sources_ = get_sources();
  init_variables(languages, project_version, project_root, release);

  if (!is_install)
    generate_compile_commands(languages);
  generate_Makefile(languages);
}

std::map<Language, std::vector<std::string>> Component::get_sources() const
{
  std::map<Language, std::vector<std::string>> sources;
  bool                                         found_any = false;

  for (const auto &src_dir : cconf.src_dirs)
  {
    const fs::path full_src_dir = root_dir / src_dir;
    if (!fs::exists(full_src_dir))
      throw ZCException(ZCE_NOT_FOUND, "Source dir " + full_src_dir.string() + " not found.");

    for (const auto &entry : fs::recursive_directory_iterator(full_src_dir))
    {
      const fs::path &file = entry.path();
      if (const Language l = language_of(file); l != UNKNOWN_LANGUAGE)
      {
        std::string rel_path = fs::relative(file, root_dir).string();
        sources[l].push_back(rel_path);
        found_any = true;
      }
    }
  }

  if (!found_any)
    throw ZCException(
      ZCE_NO_SOURCE_FILES, "No source files found for component " + root_dir.filename().string()
    );

  return sources;
}

std::string Component::get_linker() const
{
  if (sources_.contains(CXX))
    return "$(CXX_COMPILER)";
  return "$(C_COMPILER)";
}

void Component::init_variables(
  const std::map<Language, LanguageConf> &languages,
  const Version                          &project_version,
  const std::filesystem::path            &project_root,
  bool release
)
{
  variables_.clear();
  const std::string comp_name = root_dir.filename().string();
  const fs::path    cache_dir = zc_root() / ZC_CACHE_DIR;

  // --- Languages (from global PConf)
  for (const auto &[lang, conf] : languages)
  {
    string name = language_to_str(lang);
    if (!sources_.contains(lang))
      continue;

    MakeVariable compiler{ name + "_COMPILER" };
    compiler.add(conf.compiler);
    variables_.insert(compiler);

    MakeVariable flags{ name + "_FLAGS" };
    flags.add("-std=" + conf.std);
    flags.add("-MMD");
    flags.add("-MP");
    flags.add("-fdiagnostics-color=always");
    if (cconf.type == PkgType::LIB)
      flags.add("-fPIC");
    if (release)
      flags.add("-O3");
    else
      flags.add("-g");
    for (const auto &flag : conf.flags)
      flags.add(flag);
    variables_.insert(flags);

    MakeVariable objs{ name + "_OBJS" };
    for (const auto &file : sources_.at(lang))
      objs.add_no_esc(file + ".o");
    variables_.insert(objs);

    MakeVariable deps{ name + "_DEPS" };
    deps.add_make_var(name + "_OBJS:.o=.d");
    variables_.insert(deps);
  }

  // --- Macros
  MakeVariable macros{ "MACROS" };
  if (release)
  {
    macros.add_macro("ZC_RELEASE");
    macros.add_macro("NDEBUG");
  }
  else
    macros.add_macro("ZC_DEBUG");

  // Project-level macros
  macros.add_macro("ZC_MAJOR", to_string(project_version.major()));
  macros.add_macro("ZC_MINOR", to_string(project_version.minor()));
  macros.add_macro("ZC_PATCH", to_string(project_version.patch()));
  macros.add_macro("ZC_VERSION", stringify(project_version.string()));
  macros.add_macro("ZC_ROOT_DIR", stringify(project_root.string()));

  for (const auto &[m_name, value] : resolved_macros)
    macros.add_macro(m_name, value);

  // --- Include dirs, lib dirs, libs
  MakeVariable incdirs{ "INCLUDE_DIRS" };
  MakeVariable libdirs{ "LIB_DIRS" };
  MakeVariable libs{ "LIBS" };

  // Own include dirs are relative to component root (build/ is inside component)
  for (const auto &inc : cconf.include_dirs)
    incdirs.add("-I../" + inc);

  // Transitive include dirs are relative to project root (one level up from component)
  // We skip our own since they're already added above
  for (const auto &inc : resolved_include_dirs)
    if (!inc.starts_with(comp_name + "/"))
      incdirs.add("-I../../" + inc);

  // External dependencies (from resolved set)
  for (const auto &[d_name, dep] : resolved_dependencies)
  {
    if (dep.origin == "std")
    {
      if (const string &t = reg_.get_pkg(d_name).target; !t.empty())
      {
        libdirs.add("-l" + t);
        continue;
      }
      for (const auto f_list = split(get_pkg_config_flags(d_name, true), ' '); const auto &f : f_list)
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
    const fs::path pkg_dir = cache_dir / d_name / dep.version.string();
    incdirs.add("-I" + (pkg_dir / INCLUDE_DIR).string());

    const Pkg &pkg = reg_.get_pkg(d_name);
    if (pkg.type == PkgType::HEADER)
      continue;

    const fs::path lib_dir = pkg_dir / LIB_DIR;
    libdirs.add("-L" + lib_dir.string());

    if (dep.static_link)
      libs.add((lib_dir / STATIC_LIB_NAME(pkg.target)).string());
    else
    {
      libs.add("-l" + pkg.target);
      libdirs.add("-Wl,-rpath," + lib_dir.string());
    }
  }

  // Required sibling components (link against their build outputs)
  for (const auto &req : resolved_required)
  {
    CConf req_conf(root_dir / ".." / req / COMPONENT_FILE);
    if (req_conf.type == PkgType::HEADER)
      continue; // HEADER components produce no library — includes/macros already merged transitively

    const fs::path req_build = root_dir / ".." / req / BUILD_DIR;
    libdirs.add("-L" + req_build.string());
    libs.add("-l" + req_conf.target);
    libdirs.add_no_esc("-Wl,-rpath,'$$ORIGIN/../../" + req + "/" + BUILD_DIR + "'");
  }

  variables_.insert(macros);
  variables_.insert(incdirs);
  variables_.insert(libdirs);
  variables_.insert(libs);
}

// ============================================================================
// Makefile generation
// ============================================================================

void Component::Makefile_comment(std::ostringstream &mk)
{
  const string s = std::format("{:%F %T}", chrono::floor<chrono::seconds>(chrono::system_clock::now()));
  mk << "# --- This file was automatically generated by ZC\n";
  mk << "# --- Date of creation: " << s << " (UTC)\n";
  mk << "# --- !! Do not edit this file manually !!\n\n";

#ifdef DEBUG_MODE
  mk << "# Remove implicit rules to analyse the Makefile faster\n";
#endif
  mk << "MAKEFLAGS += --no-builtin-rules\n";
  mk << "MAKEFLAGS += --no-builtin-variables\n";
#ifdef DEBUG_MODE
  mk << "# Warn when undefined variables are used\n";
  mk << "MAKEFLAGS += --warn-undefined-variables\n";
  mk << "GNUMAKEFLAGS ?=\n\n"; // suppress the GNUMAKEFLAGS warning from make itself
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
  mk << "all:\n\n";
}

void Component::generate_Makefile(const std::map<Language, LanguageConf> &languages) const
{
  std::ostringstream mk;

  Makefile_comment(mk);

  // Variables
  for (const auto &v : variables_)
    mk << v.make_declaration();

  for (const auto &l : languages | std::views::keys)
    if (sources_.contains(l))
      mk << "-include $(" << language_to_str(l) << "_DEPS)\n\n";

  // Target
  if (cconf.type == PkgType::BIN)
  {
    mk << "TARGET := " << BIN_NAME(cconf.target) << "\n\n";
    mk << "all: $(TARGET)\n\n";
    mk << "$(TARGET):";
    for (const auto &l : languages | std::views::keys)
      if (sources_.contains(l))
        mk << " $(" << language_to_str(l) << "_OBJS)";
    mk << "\n\t@echo \"ZC_BIN|$@\"\n";
    mk << "\t@" << get_linker() << " -o $@ $^ $(LIB_DIRS) $(LIBS)\n\n";
  }
  else if (cconf.type == PkgType::LIB)
  {
    mk << "TARGET_STATIC := " << STATIC_LIB_NAME(cconf.target) << "\n";
    mk << "TARGET_SHARED := " << SHARED_LIB_NAME(cconf.target) << "\n\n";
    mk << "all: $(TARGET_STATIC) $(TARGET_SHARED)\n\n";

    mk << "$(TARGET_STATIC):";
    for (const auto &l : languages | std::views::keys)
      if (sources_.contains(l))
        mk << " $(" << language_to_str(l) << "_OBJS)";
    mk << "\n\t@echo \"ZC_STATIC|$@\"\n";
    mk << "\t@" << gc_.archive << " $@ $^\n\n";

    mk << "$(TARGET_SHARED):";
    for (const auto &l : languages | std::views::keys)
      if (sources_.contains(l))
        mk << " $(" << language_to_str(l) << "_OBJS)";
    mk << "\n\t@echo \"ZC_SHARED|$@\"\n";
    mk << "\t@" << get_linker() << " -shared -o $@ $^ $(LIB_DIRS) $(LIBS)\n\n";
  }

  // Rules
  mk << "clean:\n\t@rm -rf *\n\n";

  for (const auto &l : languages | std::views::keys)
  {
    if (!sources_.contains(l))
      continue;
    const string name = language_to_str(l);
    for (const auto &ext : extensions_for_language(l))
    {
      const string lower_ext = lower(ext);
      mk << "%." << lower_ext << ".o: %." << lower_ext << "\n";
      mk << "\t@" MKDIR_COMMAND " $(dir $@)\n";
      mk << "\t@echo \"ZC_COMPILE|$@\"\n";
      mk << "\t@$(" << name << "_COMPILER) $(" << name
         << "_FLAGS) $(INCLUDE_DIRS) $(MACROS) -c $< -o $@\n\n";

      if (lower_ext != ext)
      {
        mk << "%." << ext << ".o: %." << ext << "\n";
        mk << "\t@" MKDIR_COMMAND " $(dir $@)\n";
        mk << "\t@echo \"ZC_COMPILE|$@\"\n";
        mk << "\t@$(" << name << "_COMPILER) $(" << name
           << "_FLAGS) $(INCLUDE_DIRS) $(MACROS) -c $< -o $@\n\n";
      }
    }
  }

  std::ofstream out(build_dir / MAKEFILE);
  out << mk.str();
}

void Component::generate_compile_commands(const std::map<Language, LanguageConf> &languages) const
{
  json compile_commands = nlohmann::json::array();

  string includes;
  string macros_str;
  if (auto it = variables_.find("INCLUDE_DIRS"); it != variables_.end())
    includes = it->string();
  if (auto it = variables_.find("MACROS"); it != variables_.end())
    macros_str = it->string();

  for (const auto &[lang, conf] : languages)
  {
    if (!sources_.contains(lang))
      continue;
    string flags;
    if (auto it = variables_.find(language_to_str(lang) + "_FLAGS"); it != variables_.end())
      flags = it->string();

    for (const auto &file : sources_.at(lang))
    {
      nlohmann::json cmd;
      cmd["directory"] = fs::absolute(build_dir).string();
      cmd["file"]      = "../" + file;
      cmd["output"]    = file + ".o";
      cmd["command"] =
        join({ conf.compiler, flags, includes, macros_str, "-c", "../" + file, "-o", file + ".o" });
      compile_commands.push_back(cmd);
    }
  }

  std::ofstream out(build_dir / "compile_commands.json");
  out << compile_commands.dump(2);
}

// ============================================================================
// Dependency management (passthrough to CConf)
// ============================================================================

void Component::add_dependency(const LocalTarget &target, bool is_static)
{
  Dependency dep{
    .name        = target.name,
    .origin      = target.origin,
    .static_link = is_static,
    .version     = target.version,
  };

  if (reg_.get_pkg(dep.name).type == PkgType::BIN)
    throw ZCException(ZCE_TYPE_ERROR, "Cannot add dependency of type BIN");

  cconf.add_dependency(dep);
}

void Component::remove_dependency(const std::string &name)
{
  cconf.remove_dependency(name);
}

void Component::change_dependency_version(const std::string &name, const Version &new_version)
{
  if (!reg_.is_installed(name, new_version))
    throw ZCException(ZCE_PKG_NOT_FOUND, "Package '" + name + "' is not installed");
  cconf.change_dependency_version(name, new_version);
}

void Component::install_dependencies(bool force) const
{
  for (const auto &dep : cconf.dependencies | std::views::values)
  {
    if (dep.origin == "local")
      ui().warning("Dependency '" + dep.name + "' is a local package. Make sure it's installed.");
    else if (dep.origin == "std")
      reg_.install_std(dep.name, force);
    else
    {
      RemoteTarget t = RemoteTarget::get_target({ dep.name, dep.version });
      reg_.install_from_server(t, force);
    }
  }
}

void Component::uninstall_dependencies(bool force) const
{
  for (const auto &dep : cconf.dependencies | std::views::values)
  {
    if (dep.origin == "std")
    {
      ui().warning("Skipped dependency '" + dep.name + "' which is a standard package.");
      continue;
    }
    LocalTarget t = LocalTarget::get_target({ dep.name, dep.version });
    reg_.uninstall(t, force);
  }
}

void Component::update_dependencies(bool force, bool use)
{
  for (const auto &dep : cconf.dependencies | std::views::values)
  {
    if (dep.origin == "std")
      continue;
    if (dep.origin == "local")
    {
      if (auto path = reg_.get_pkg(dep.name).path; !path.empty() && fs::exists(path))
      {
        // Local update would require a Project, skip for now
        ui().warning("Local dependency update for components not yet supported: " + dep.name);
      }
    }
    else
    {
      RemoteTarget t = RemoteTarget::get_target({ dep.name, Version::latest() });
      reg_.update_from_server(t, force, use);
      cconf.change_dependency_version(t.name, t.version);
    }
  }
}

} // namespace zc
