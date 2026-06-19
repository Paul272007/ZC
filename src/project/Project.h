/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../config/PConf.h"
#include "../pkgs/Registry.h"
#include "Language.h"
#include "ui/Interface.h"

namespace zc
{

using Sources = std::map<Language, std::vector<std::string>>;

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
   * @param release
   * @param force
   */
  void build(bool release = false);

  void clean() const;

  void publish();

  /**
   * @param name
   */
  void add_dependency(const std::string &name);

  /**
   * @param name
   */
  void remove_dependency(const std::string &name);

  void install_dependencies() const;

  void generate_build_config();

private:
  const std::filesystem::path cache_dir_;
  const std::filesystem::path makefile_;
  Sources sources_;
  Registry &reg_ = Registry::get();
  Interface &if_ = Interface::get();

  void generate_Makefile(bool release = false);

  void Makefile_bin(std::ostringstream &mk) const;
  void Makefile_lib(std::ostringstream &mk) const;
  void Makefile_compose(std::ostringstream &mk) const;

  void Makefile_comment(std::ostringstream &mk) const;
  void Makefile_variables(std::ostringstream &mk, bool release) const;
  void Makefile_rules(std::ostringstream &mk) const;

  void generate_compile_commands() const;

  void get_sources();
};

} // namespace zc
