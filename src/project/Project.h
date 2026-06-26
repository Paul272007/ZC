#pragma once

#include <filesystem>
#include <set>
#include <string>

#include "../config/GConf.h"
#include "../config/PConf.h"
#include "../excepts/ExitCode.h"
#include "../excepts/ZCException.h"
#include "../helpers.h"
#include "../Language.h"
#include "../pkgs/Registry.h"
#include "../ui/Interface.h"
#include "MakeVariable.h"

namespace zc
{

using Sources = std::map<Language, std::vector<std::string>>;

enum class BuildMode
{
  automatic,
  release,
  debug,
};

inline std::string build_mode_to_str(BuildMode mode)
{
  switch (mode)
  {
  case BuildMode::release:
    return "release";
  case BuildMode::debug:
    return "debug";
  default:
    return "";
  }
}

inline BuildMode build_mode_from_str(const std::string &str)
{
  const auto upper_str = upper(str);
  if (upper_str == "RELEASE")
    return BuildMode::release;
  if (upper_str == "DEBUG")
    return BuildMode::debug;
  throw ZCException(ZCE_CONTENT_ERROR, "Invalid build mode declaration.");
}

class Project
{
public:
  // expose configuration for the registry
  const std::filesystem::path root_dir;
  const std::filesystem::path build_dir;

  PConf pconf;

  explicit Project(const std::filesystem::path &root = get_project_root());

  ~Project() = default;

  void build(BuildMode current_mode = BuildMode::debug, bool is_install = false);

  void clean(bool cache = false) const;

  void publish();

  void add_dependency(const std::string &name, bool is_static = false);
  void remove_dependency(const std::string &name);
  void change_dependency_version(const std::string &name, const Version &new_version);

  void install_dependencies() const;
  void update_dependencies();

  void generate_build_config();

private:
  GConf     &gc_  = GConf::get();
  Registry  &reg_ = Registry::get();
  Interface &if_  = Interface::get();

  const std::filesystem::path cache_dir_;
  const std::filesystem::path makefile_;

  std::set<MakeVariable, MakeVariableCmp> variables_;

  Sources sources_;

  void generate_Makefile(bool release = false) const;

  void Makefile_bin(std::ostringstream &mk) const;
  void Makefile_lib(std::ostringstream &mk) const;
  void Makefile_compose(std::ostringstream &mk) const;

  void Makefile_comment(std::ostringstream &mk) const;
  void Makefile_variables(std::ostringstream &mk, bool release) const;
  void Makefile_rules(std::ostringstream &mk) const;

  void generate_compile_commands() const;

  int get_sources();
  std::string get_linker() const;
  void init_variables(bool release);
};

} // namespace zc
