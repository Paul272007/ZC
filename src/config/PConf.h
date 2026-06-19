/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <string>
#include <vector>

#include "../Version.h"
#include "../helpers.h"
#include "../pkgs/PkgType.h"
#include "Conf.h"
#include "Dependency.h"
#include "LanguageConf.h"

namespace zc
{

class PConf : public Conf
{
public:
  std::string name;
  std::string author;
  std::string target = name;
  PkgType type = UNDEF;
  Version version = {0, 0, 0};
  std::vector<std::string> src_dirs = {SRC_DIR};
  std::vector<std::string> include_dirs = {SRC_DIR};
  std::vector<Dependency> dependencies;
  std::vector<LanguageConf> languages = {
      {C, "c17", "clang", {"-Wall", "-Wextra"}}, {CXX, "c++20", "clang++", {"-Wall", "-Wextra"}}
  };

  ~PConf() override;

  void add_dependency(const Dependency &d);

  void remove_dependency(const std::string &dep_name);

  LanguageConf get_lang_conf(Language l) const;

  /**
   * @param file
   */
  explicit PConf(const std::filesystem::path &file = get_project_root() / ZC_FILE);

protected:
  void load() override;

  void write() override;
};

} // namespace zc
