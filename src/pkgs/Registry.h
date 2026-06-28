#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../config/Conf.h"
#include "../config/Dependency.h"
#include "../helpers.h"
#include "../ui/Table.h"
#include "../Version.h"
#include "Network.h"
#include "Pkg.h"

namespace zc
{

class Project;

class Registry : public Conf
{
public:
  /**
   * Get the user registry, which index is stored in ~/.zc/registry.json
   */
  [[nodiscard]] static Registry &get();
  Registry(const Registry &)       = delete;
  void operator=(const Registry &) = delete;

  /**
   * @param name
   */
  Pkg get_pkg(const std::string &name);

  Dependency get_dependency(const Target &target);

  Version get_latest(const std::string &name);

  void install_std(const std::string &name);

  /**
   * Install a package from the server
   * If already installed, throw an error
   * @param target
   * @param index
   * @param force
   */
  void install_from_server(Target &target, const nlohmann::json &index, bool force = false);

  /**
   * Install a local package globally
   * @param path The path to the project root
   * @param force Whether to force installation even if the package is already installed
   */
  void install_from_path(const std::filesystem::path &path, bool force = false);

  void update_from_server(Target &target, const nlohmann::json &index, bool force = false);

  /**
   * @brief Update a project from local path
   *
   * @param path The path to the project to update
   * @param force Whether to force update
   * @return The created project at the root of the project
   */
  Project update_from_path(const std::filesystem::path &path, bool force = false);

  /**
   * Uninstall a package and all its versions
   * @param pkg
   */
  void uninstall(const std::string &pkg);

  /**
   * Check if target is installed
   * @param target
   */
  [[nodiscard]] bool is_installed(const Target &target);

  [[nodiscard]] bool is_installed(const std::string &name) const;

  [[nodiscard]] std::map<std::string, Pkg> pkgs() const;

  [[nodiscard]] Table pkgs_table() const;

  [[nodiscard]] std::vector<std::string> remote_pkgs() const;

  [[nodiscard]] Table remote_pkgs_table() const;

  ~Registry() override;

protected:
  void load() override;
  void write() override;

private:
  const Network &net_ = Network::get();

  const std::filesystem::path cache_dir_;
  const std::filesystem::path tmp_dir_;
  const std::filesystem::path include_links_dir_;
  const std::filesystem::path lib_links_dir_;
  const std::filesystem::path bin_links_dir_;

  std::map<std::string, Pkg> pkgs_; // pkg name -> pkg declaration

  explicit Registry(const std::filesystem::path &root = zc_root());

  /**
   * @param pkg
   */
  void index_add_pkg(const Pkg &pkg);

  void index_add_pkg_version(const std::string &name, const Version &version);

  [[nodiscard]] std::map<std::string, Pkg>::iterator get_pkg_it(const std::string &name);

  /**
   * Remove pkg from index. A package with the same name must be found in the registry index.
   * @param name
   */
  Pkg unindex_pkg(const std::string &name);

  void finish_install(Project &p, const std::string &origin);
  void finish_update(Project &p);

  void copy_bin(const Project &p) const;

  void copy_headers(const Project &p) const;

  void copy_libs(const Project &p) const;

  /**
   * @brief Remove tmp dir
   */
  void clean() const;

  /**
   * Download archive, verify its hash, extract it and return the root of the project
   */
  std::filesystem::path download_and_extract(const Target &target, const nlohmann::json &index) const;

  /**
   * @param archive
   * @param expected
   */
  void verify_archive_hash(const std::filesystem::path &archive, const std::string &expected) const;

  [[nodiscard]] static std::string pkg_url(const Target &target, const nlohmann::json &index);

  static void verify_headers_structure(const Project &p);
};

} // namespace zc
