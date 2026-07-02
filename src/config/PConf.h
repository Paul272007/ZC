#pragma once

#include <map>
#include <string>

#include "config/Conf.h"
#include "config/Dependency.h"
#include "config/Language.h"
#include "config/LanguageConf.h"
#include "helpers.h"
#include "pkgs/PkgType.h"
#include "Version.h"

namespace zc
{

class PConf : public Conf
{
public:
  std::string name;
  std::string author;
  std::string target = name;

  PkgType type    = PkgType::UNDEF;
  Version version = { 0, 0, 1 };

  std::vector<std::string> src_dirs     = { SRC_DIR };
  std::vector<std::string> include_dirs = { SRC_DIR };

  std::map<std::string, std::string> macros;
  std::map<std::string, Dependency>  dependencies;
  std::map<Language, LanguageConf>   languages;

  void add_dependency(const Dependency &d);
  void remove_dependency(const std::string &dep_name);
  void change_dependency_version(const std::string &dep_name, const Version &new_version);

  [[nodiscard]] bool has_language(Language l) const;
  void add_language(Language l);
  void remove_language(Language l);
  void edit_language(Language l);
  void edit_languages();

  ~PConf() override;
  explicit PConf(const std::filesystem::path &file = get_project_root() / ZC_FILE);

  PConf(const PConf &)            = delete;
  PConf(PConf &&)                 = default;
  PConf &operator=(const PConf &) = delete;
  PConf &operator=(PConf &&)      = delete;

protected:
  void load() override;
  void write() override;
};

} // namespace zc
