/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _PCONF_H
#define _PCONF_H

#include <string>
#include <vector>

#include "../Language.h"
#include "../Version.h"
#include "../pkgs/PkgType.h"
#include "Conf.h"
#include "Dependency.h"

class PConf : public Conf
{
public:
  std::string name;
  std::string author;
  std::string target = name;
  std::string c_std = "c23";
  std::string cxx_std = "c++20";
  std::string c_compiler = "clang";
  std::string cxx_compiler = "clang++";
  PkgType type = UNDEF;
  Version version = {0, 0, 0};
  std::vector<std::string> flags = {"-Wall", "-Wextra"};
  std::vector<Dependency> dependencies;
  std::vector<Language> languages;

  PConf();

  /**
   * @param file
   */
  PConf(const std::filesystem::path &file);

protected:
  void load();

  void write();
};

#endif //_PCONF_H
