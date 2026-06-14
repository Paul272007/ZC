/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _PROJECT_H
#define _PROJECT_H

#include <string>

#include "../config/PConf.h"
#include "../pkgs/Registry.h"

class Project
{
public:
  // expose configuration for the registry
  const std::filesystem::path root_dir;
  const std::filesystem::path build_dir;
  PConf pconf;

  explicit Project(const std::filesystem::path &root);

  ~Project() = default;

  /**
   * @param release
   */
  void build(bool release = false, bool force = false);

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

private:
  const std::filesystem::path makefile_;
  Registry &reg_;
  Interface &if_;

  void generate_Makefile();

  void generate_compile_commands();
};

#endif //_PROJECT_H
