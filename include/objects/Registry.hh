#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "objects/Table.hh"
#include "objects/Version.hh"

struct Package
{
  std::string name;
  std::string binary;
  std::string origin;
  Version version = {0, 0, 0};
  bool is_exec;
  bool is_installed_locally = true;
};

class Registry
{
public:
  explicit Registry(const std::filesystem::path &file) : file_(file)
  {
  }
  virtual ~Registry() = default;

  void write() const;
  void indexPackage(const Package &package);
  Package unindexPackage(const std::string &pkg_name);

  [[nodiscard]] const Package &getPackage(const std::string &pkg_name) const;

  [[nodiscard]] bool pkgExists(const std::string &pkg_name) const;

  [[nodiscard]] Table packagesTable() const;
  [[nodiscard]] const std::vector<Package> &getPackages() const;

protected:
  void load();

  std::vector<Package> pkgs_;
  const std::filesystem::path file_;
};
