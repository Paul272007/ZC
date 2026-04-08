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

  std::vector<std::string> headers_;
  std::vector<std::string> binaries_;

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
   * @brief Save a library based on its configuration and its files (headers /
   * object files)
   * 1. Create a subdirectory in the ZC include dir
   * 2. Copy the header files into this directory
   * 3. Compile the object files into a static and dynamic library
   * 4. Put the binaries into the lib directory
   *
   * @param package The package configuration
   * @param force Force installation even if the library already exists
   * @param headers The library's header files
   * @param objects The library's object files
   * @param sources The library's source files
   * @param is_cpp Whether library is C++ or not
   */
  void savePackage(
      Package &package, bool force,
      const std::vector<std::filesystem::path> &headers,
      std::vector<std::filesystem::path> &objects,
      const std::vector<std::filesystem::path> &sources, bool is_cpp
  );

  /**
   * @brief Uninstall package and remove it from index
   *
   * @param pkg_name The target package
   * @return Whether it was successful
   */
  bool removePackage(const std::string &pkg_name);

  [[nodiscard]] bool pkgExists(const std::string &pkg_name) const;

  /**
   * @brief Get all the packages of the registry
   */
  [[nodiscard]] std::vector<Package> getPackages() const;

  /**
   * @brief Get all the packages of the registry
   */
  [[nodiscard]] std::vector<StdPackage> getStdPackages() const;

  /**
   * @brief Get the include path
   */
  [[nodiscard]] std::filesystem::path getIncludeDir() const;

  /**
   * @brief Get the binaries path
   */
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
  std::vector<std::string> unindexPackage(const std::string &pkg_name);

  /**
   * @brief Compile source files to object files
   *
   * @param sources The source files to be compiled (.c, .i, .s)
   * @param objects The vector that is going to contain the compiled objects
   * @param is_cpp Whether it is C++
   */
  static void compileObjects(
      const std::vector<std::filesystem::path> &sources,
      std::vector<std::filesystem::path> &objects, bool is_cpp
  );

  /**
   * @brief Create a static library
   *
   * @param libPath The path of the future binary
   * @param objects The objects to be compiled
   * @return whether it worked or not
   */
  static bool createStaticLib(
      const std::string &libPath,
      const std::vector<std::filesystem::path> &objects
  );

  /**
   * @brief Create a shared library
   *
   * @param libPath The path of the future library
   * @param objects The objects to be compiled
   * @param is_cpp Whether the code is C++
   * @return whether it worked or not
   */
  static bool createSharedLib(
      const std::string &libPath,
      const std::vector<std::filesystem::path> &objects, bool is_cpp
  );

  std::vector<Package> packages_;
  std::vector<StdPackage> std_packages_;

  std::filesystem::path registry_path_ = getZCRootDir() / REGISTRY;

  std::filesystem::path include_path_ = getZCRootDir() / "include";
  std::filesystem::path lib_path_ = getZCRootDir() / "lib";
};