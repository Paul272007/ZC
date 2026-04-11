#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <helpers.hh>
#include <objects/Table.hh>

#define REGISTRY "registry.json"
#define N_ATTR_PACKAGE 6
#define N_ATTR_STD_PACKAGE 4

struct Package
{
  std::string name_;
  std::string author_;
  std::string version_;
  std::string flags_;
};

struct StdPackage
{
  std::string name_;
  std::vector<std::string> headers_;
  std::vector<std::string> binaries_;
  std::string flags_;
};

class Registry
{
public:
  /**
   * @brief Get an instance
   *
   * There can't be more than one instance at a time
   *
   * @return A Registry instance
   */
  static Registry &getInstance();

  /**
   * @brief Load the Registry's content
   */
  void load();

  /**
   * @brief install a library based on its root folder
   *
   * @param project_root The root of the project
   * @param force Force installation even if the library already exists
   * @param quiet Activate quiet mode
   */
  void installPackage(std::filesystem::path &project_root, const bool force, const bool quiet);

  /**
   * @brief Uninstall package and remove it from index
   *
   * @param pkg_name The target package
   * @return Whether it was successful
   */
  bool removePackage(const std::string &pkg_name);

  [[nodiscard]] bool pkgExists(const std::string &pkg_name) const;

  [[nodiscard]] std::vector<Package> getPackages() const;

  [[nodiscard]] std::vector<StdPackage> getStdPackages() const;

  [[nodiscard]] std::filesystem::path getIncludeDir() const;

  [[nodiscard]] std::filesystem::path getLibDir() const;

  /**
   * @brief Create a Table containing all the packages, ready to be displayed
   *
   * @return The Table
   */
  [[nodiscard]] Table packagesTable() const;

  /**
   * @brief Create a Table containing all the standard packages, ready to be
   * displayed
   *
   * @return The Table
   */
  [[nodiscard]] Table stdPackagesTable() const;

private:
  /**
   * @brief Default constructor
   */
  Registry();

  /**
   * @brief Index the Package in the configuration file
   */
  void indexPackage(const Package &package);

  /**
   * @brief Unindex package from registry
   *
   * @param pkg_name The name of the package to be unindexed
   */
  void unindexPackage(const std::string &pkg_name);

  /**
   * @brief Write to ~/.zc/registry.json
   */
  void write() const;

  std::vector<Package> packages_;
  std::vector<StdPackage> std_packages_;

  std::filesystem::path registry_path_ = getZCRootDir() / REGISTRY;

  std::filesystem::path include_path_ = getZCRootDir() / "include";
  std::filesystem::path lib_path_ = getZCRootDir() / "lib";
};
