#pragma once

#include <filesystem>
#include <string>

#include <helpers.hh>
#include <objects/Table.hh>

#define N_ATTR_PKG 5

class Build;

struct Package
{
  std::string name_;
  std::string version_;
  std::string binary_;
  std::string origin_;
  bool is_bin_;
};

class Registry
{
public:
  Registry(bool is_global, const std::filesystem::path &project_root = getProjectRoot());
  void write() const;
  /**
   * @brief Install a library based on its root folder
   *
   * @param project_root The root of the project to install
   * @param force Force installation even if the library already exists
   * @param quiet Activate quiet mode
   */
  void installPackage(
      const std::filesystem::path &project_root, bool force, bool quiet, const std::string &origin
  );

  /**
   * @brief Uninstall package and remove it from index
   *
   * @param pkg_name The target package
   * @return Whether all files were successfully deleted or not
   */
  bool removePackage(const std::string &pkg_name);

  const Package &getPackage(const std::string &pkg_name) const;

  /**
   * @param pkg_name The name of the package to check
   * @return Whether the package was found or not
   */
  [[nodiscard]] bool pkgExists(const std::string &pkg_name) const;

  void indexPackage(const Package &package);

  /**
   * @brief Create a Table containing all the packages, ready to be displayed
   *
   * @return The Table
   */
  [[nodiscard]] Table packagesTable() const;
  [[nodiscard]] const std::vector<Package> &getPackages() const;
  [[nodiscard]] const std::filesystem::path &getIncludePath() const;
  [[nodiscard]] const std::filesystem::path &getLibPath() const;

private:
  void load();
  void installExecutable(const Build &b, bool quiet);
  void installLibrary(const Build &b, bool quiet);

  Package unindexPackage(const std::string &pkg_name);

  std::filesystem::path registry_path_;
  std::filesystem::path include_path_;
  std::filesystem::path lib_path_;
  std::filesystem::path bin_path_;
  std::vector<Package> pkgs_;
};
