#pragma once

#include <map>
#include <string>
#include <vector>

#include "../helpers.h"
#include "../Language.h"
#include "../pkgs/PkgType.h"
#include "../Version.h"
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

  PkgType type = PkgType::UNDEF;
  Version version; // default is 0.0.1

  std::vector<std::string>         src_dirs     = { SRC_DIR };
  std::vector<std::string>         include_dirs = { SRC_DIR };
  std::vector<Dependency>          dependencies;
  std::map<Language, LanguageConf> languages;

  ~PConf() override;

  void add_dependency(const Dependency &d);
  void remove_dependency(const std::string &dep_name);
  void change_dependency_version(const std::string &name, const Version &new_version);

  explicit PConf(const std::filesystem::path &file = get_project_root() / ZC_FILE);

protected:
  void load() override;

  void write() override;
};

} // namespace zc
