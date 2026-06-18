/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <string>

#include "../config/PConf.h"
#include "../pkgs/Registry.h"

namespace zc
{

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
  void build(bool release = false) const;

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

  void generate_build_config() const;

private:
  const std::filesystem::path src_dir_;
  const std::filesystem::path makefile_;
  Registry &reg_;
  Interface &if_;

  void generate_Makefile() const;

  void Makefile_bin(std::ostringstream &mk) const;
  void Makefile_lib(std::ostringstream &mk) const;
  void Makefile_compose(std::ostringstream &mk) const;

  void generate_compile_commands() const;

  void get_sources(std::vector<std::string> &c_files, std::vector<std::string> &cxx_files) const;
};

} // namespace zc
