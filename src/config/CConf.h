#pragma once

#include <map>
#include <string>

#include "config/Conf.h"
#include "config/Dependency.h"
#include "helpers.h"
#include "pkgs/PkgType.h"
#include "ui/Table.h"
#include "Version.h"

namespace zc
{

class CConf : public Conf
{
public:
  PkgType     type{ PkgType::UNDEF };
  std::string target;

  std::vector<std::string> src_dirs;
  std::vector<std::string> include_dirs;
  std::vector<std::string> required;

  std::map<std::string, std::string> macros;
  std::map<std::string, Dependency>  dependencies;

  void add_dependency(const Dependency &d);
  void remove_dependency(const std::string &dep_name);
  void change_dependency_version(const std::string &dep_name, const Version &new_version);

  [[nodiscard]] Table dependencies_table() const;

  ~CConf() override;
  explicit CConf(const std::filesystem::path &file);

  CConf(const CConf &)            = delete;
  CConf(CConf &&)                 = default;
  CConf &operator=(const CConf &) = delete;
  CConf &operator=(CConf &&)      = delete;

protected:
  void load() override;
  void write() override;
};

} // namespace zc
