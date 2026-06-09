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
  Project();

  ~Project();

  /**
   * @param release
   */
  void build(bool release);

  void clean();

  void publish();

  /**
   * @param target
   */
  bool add_dependency(std::string target);

  /**
   * @param target
   */
  bool remove_dependency(std::string target);

private:
  const std::filesystem::path root_dir_;
  const std::filesystem::path build_dir_;
  const std::filesystem::path makefile_;
  Registry &reg_;
  PConf pconf_;

  void generate_Makefile();

  void generate_compile_commands();
};

#endif //_PROJECT_H
