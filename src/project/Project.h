/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../Language.h"
#include "../config/GConf.h"
#include "../config/PConf.h"
#include "../excepts/ExitCode.h"
#include "../excepts/ZCException.h"
#include "../helpers.h"
#include "../pkgs/Registry.h"
#include "../ui/Interface.h"

namespace zc
{

using Sources = std::map<Language, std::vector<std::string>>;

enum class BuildMode
{
  release,
  debug
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

  /**
   * @param current_mode
   * @param force
   */
  void build(BuildMode current_mode = BuildMode::debug);

  void clean() const;

  void publish();

  void add_dependency(const std::string &name);
  void remove_dependency(const std::string &name);
  void change_dependency_version(const std::string &name, const Version &new_version);

  void install_dependencies() const;
  void update_dependencies() const;

  void generate_build_config();

private:
  const std::filesystem::path cache_dir_;
  const std::filesystem::path makefile_;
  Sources sources_;
  Registry &reg_ = Registry::get();
  Interface &if_ = Interface::get();
  GConf &gc_ = GConf::get();
  int to_compile_ = 0;

  void generate_Makefile(bool release = false);

  void Makefile_bin(std::ostringstream &mk) const;
  void Makefile_lib(std::ostringstream &mk) const;
  void Makefile_compose(std::ostringstream &mk) const;

  void Makefile_comment(std::ostringstream &mk) const;
  void Makefile_variables(std::ostringstream &mk, bool release) const;
  void Makefile_rules(std::ostringstream &mk) const;

  void generate_compile_commands() const;

  int get_sources();
  std::string get_linker() const;
};

} // namespace zc
