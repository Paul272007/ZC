#pragma once

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "config/CConf.h"
#include "config/Dependency.h"
#include "config/GConf.h"
#include "config/Language.h"
#include "config/LanguageConf.h"
#include "pkgs/LocalTarget.h"
#include "pkgs/Registry.h"
#include "project/MakeVariable.h"

namespace zc
{

class Component
{
public:
  const std::filesystem::path root_dir;
  const std::filesystem::path build_dir;

  CConf cconf;

  explicit Component(std::filesystem::path root);

  // --- Build
  void generate_build_config(
    const std::map<Language, LanguageConf> &languages,
    const Version                          &project_version,
    const std::filesystem::path            &project_root,
    bool release, bool is_install
  );
  void generate_Makefile(const std::map<Language, LanguageConf> &languages) const;
  void generate_compile_commands(const std::map<Language, LanguageConf> &languages) const;

  // --- Transitive dependency resolution
  /// @brief Resolve transitive dependencies from sibling components.
  ///        Populates resolved_* members with merged includes, macros, deps, and required components.
  void resolve_dependencies(
    const std::map<std::string, Component> &all_components, std::unordered_set<std::string> &visited
  );

  // --- Dependency management
  void add_dependency(const LocalTarget &target, bool is_static = false);
  void remove_dependency(const std::string &name);
  void change_dependency_version(const std::string &name, const Version &new_version);

  void install_dependencies(bool force) const;
  void uninstall_dependencies(bool force) const;
  void update_dependencies(bool force, bool use);

  // --- Resolved properties (populated by resolve_dependencies)
  std::vector<std::string>              resolved_include_dirs;
  std::vector<std::string>              resolved_required;
  std::map<std::string, std::string>    resolved_macros;
  std::map<std::string, Dependency>     resolved_dependencies;

private:
  GConf    &gc_  = GConf::get();
  Registry &reg_ = Registry::get();

  std::set<MakeVariable, MakeVariableCmp>      variables_;
  std::map<Language, std::vector<std::string>>  sources_;

  [[nodiscard]] std::map<Language, std::vector<std::string>> get_sources() const;
  [[nodiscard]] std::string get_linker() const;

  void init_variables(
    const std::map<Language, LanguageConf> &languages,
    const Version                          &project_version,
    const std::filesystem::path            &project_root,
    bool release
  );

  static void Makefile_comment(std::ostringstream &mk);
};

} // namespace zc
