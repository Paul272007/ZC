#pragma once

#include <cstddef>
#include <filesystem>
#include <set>
#include <string>
#include <vector>

#include "config/GConf.h"
#include "config/PConf.h"
#include "pkgs/Registry.h"
#include "project/BuildMode.h"
#include "project/Component.h"
#include "project/MakeVariable.h"

namespace zc
{

class ShellCommand;
struct LocalTarget;
enum Language : std::uint8_t;

class Project
{
public:
  // expose configuration for the registry
  const std::filesystem::path root_dir;
  const std::filesystem::path build_dir;

  PConf pconf;

  explicit Project(const std::filesystem::path &root = get_project_root());
  Project(const Project &)            = delete;
  Project(Project &&)                 = default;
  Project &operator=(const Project &) = delete;
  Project &operator=(Project &&)      = delete;
  ~Project()                          = default;

  void generate_build_config(BuildMode current_mode = BuildMode::debug, bool is_install = false);
  void build(
    BuildMode current_mode = BuildMode::automatic, bool is_install = false, size_t jobs = 1,
    const std::string &target = "all"
  );
  void clean(bool cache = false) const;
  void publish();
  void execute(const std::vector<std::string> &args) const;

  void add_dependency(const LocalTarget &target, bool is_static = false);
  void remove_dependency(const std::string &name);
  void change_dependency_version(const std::string &name, const Version &new_version);

  void install_dependencies(bool force) const;
  void uninstall_dependencies(bool force) const;
  void update_dependencies(bool force, bool use);

private:
  GConf    &gc_  = GConf::get();
  Registry &reg_ = Registry::get();

  const std::filesystem::path cache_dir_;
  const std::filesystem::path makefile_;

  std::set<MakeVariable, MakeVariableCmp>      variables_; // Each make variable with its name and value
  std::map<Language, std::vector<std::string>> sources_; // For each language we have a list of source files
  std::map<std::string, Component>             components_; // Components of a COMPOSE package

  void generate_compile_commands() const;
  void generate_Makefile() const;

  /// @brief Write part of the Makefile relative to the BIN type
  void Makefile_bin(std::ostringstream &mk) const;
  /// @brief Write part of the Makefile relative to the LIB type
  void Makefile_lib(std::ostringstream &mk) const;
  /// @brief Write part of the Makefile relative to the COMPOSE type
  void Makefile_compose(std::ostringstream &mk) const;
  /// @brief Write variables_ into Makefile
  void Makefile_variables(std::ostringstream &mk) const;
  /// @brief Write rules to build selected languages
  void Makefile_rules(std::ostringstream &mk) const;
  /// @brief write ZC comment + Make boilerplate into Makefile
  static void Makefile_comment(std::ostringstream &mk);

  [[nodiscard]] BuildMode get_mode(BuildMode current_mode) const;
  [[nodiscard]] std::string get_linker() const;
  [[nodiscard]] std::map<Language, std::vector<std::string>> get_sources() const;

  void load_components();

  /// @brief Initialize configuration for the Makefile
  void init_variables(bool release);
  /// @brief Initialize languages configuration
  void init_languages(bool release);

  static void get_nb_to_compile(int &to_compile, int &to_link, ShellCommand base_make_cmd);
};

} // namespace zc
