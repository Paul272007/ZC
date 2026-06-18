/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <string>
#include <vector>

#include "../config/Conf.h"
#include "../ui/Table.h"
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
  Registry(const Registry &) = delete;
  void operator=(const Registry &) = delete;

  /**
   * @param name
   */
  RegistryPkg get_pkg(const std::string &name);

  /**
   * Install a package from the server
   * If already installed, throw an error
   * TODO : add latest as default version here and make version a Version + use Target struct
   * @param name
   * @param version
   * @param index
   * @param force
   */
  void install_from_server(
      const std::string &name, const std::string &version, const nlohmann::json &index, bool force = false
  );

  /**
   * Install a local package globally
   * @param path The path to the project root
   * @param force Whether to force installation even if the package is already installed
   */
  void install_from_path(const std::filesystem::path &path, bool force = false);

  /**
   * Add a new version to a package (latest available version by default)
   * @param name
   * @param version
   * @param index
   * @param force
   */
  void update_from_server(
      const std::string &name, const std::string &version, const nlohmann::json &index, bool force = false
  );

  void update_from_path(const std::filesystem::path &path, bool force = false);

  /**
   * Uninstall a package and all its versions
   * @param pkg
   */
  void uninstall(const std::string &pkg);

  /**
   * Check if pkg is installed
   * @param name
   * @param version
   */
  [[nodiscard]] bool is_installed(const std::string &name, const Version &version);

  [[nodiscard]] bool is_installed(const std::string &name);

  [[nodiscard]] std::vector<RegistryPkg> pkgs() const;

  [[nodiscard]] Table pkgs_table() const;

  [[nodiscard]] std::vector<std::string> remote_pkgs() const;

  [[nodiscard]] Table remote_pkgs_table() const;

  ~Registry() override;

protected:
  void load() override;

  void write() override;

private:
  const std::filesystem::path cache_dir_;
  /* ! useless for the moment ! */
  const std::filesystem::path bin_dir_;
  const std::filesystem::path lib_dir_;
  const std::filesystem::path include_dir_;
  /* */
  const std::filesystem::path bin_links_dir_;
  const std::filesystem::path tmp_dir_;
  std::vector<RegistryPkg> pkgs_;
  const Network &net_;

  /**
   * @param root
   */
  explicit Registry(const std::filesystem::path &root);

  /**
   * @param pkg
   */
  void index_add_pkg(const RegistryPkg &pkg);

  void index_add_pkg_version(const std::string &name, const Version &version);

  [[nodiscard]] std::vector<RegistryPkg>::iterator get_pkg_it(const std::string &name);

  /**
   * Remove pkg from index. A package with the same name must be found in the registry index.
   * @param name
   */
  RegistryPkg unindex_pkg(const std::string &name);

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
  std::filesystem::path download_and_extract(
      const std::string &name, const std::string &version, const nlohmann::json &index
  ) const;

  /**
   * @param archive
   * @param expected
   */
  void verify_archive_hash(const std::filesystem::path &archive, const std::string &expected) const;

  [[nodiscard]] static std::string
  pkg_url(const std::string &name, const std::string &version, const nlohmann::json &index);
};

} // namespace zc
