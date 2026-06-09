/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _REGISTRY_H
#define _REGISTRY_H

#include <string>
#include <vector>

#include "../config/Conf.h"
#include "../helpers.h"
#include "Network.h"
#include "Pkg.h"

class Registry : public Conf
{
public:
  /**
   * Get the user registry : ~/.zc/registry.json
   */
  static Registry &get();

  /**
   * Uninstall a package
   * @param pkg
   */
  bool remove_pkg(const std::string &pkg);

  /**
   * @param name
   */
  Pkg get_pkg(const std::string &pkg);

  std::vector<Pkg> pkgs();

  /**
   * @param name
   * @param version
   */
  void install_pkg(const std::string &name, Version version);

  /**
   * @param name
   */
  void update_pkg(const std::string &name);

protected:
  void load();

  void write();

private:
  const std::filesystem::path root_dir_ = get_zc_root();
  const std::filesystem::path bin_dir_ = root_dir_ / CACHE_DIR / BIN_DIR;
  const std::filesystem::path lib_dir_ = root_dir_ / CACHE_DIR / LIB_DIR;
  const std::filesystem::path include_dir_ = root_dir_ / CACHE_DIR / INCLUDE_DIR;
  const std::filesystem::path tmp_dir_ = root_dir_ / TMP_DIR;
  std::vector<Pkg> pkgs_;
  Network &net_;

  /**
   * @param root
   */
  Registry(const std::filesystem::path &root);

  /**
   * @param pkg
   */
  void index_pkg(const Pkg &pkg);

  /**
   * Remove pkg from index. A package with the same name and version must be found in the registry index.
   * @param pkg
   */
  void unindex_pkg(const Pkg &pkg);

  /**
   * Check if pkg is installed
   * @param pkg
   */
  bool is_installed(const std::string &name);

  void copy_bin();

  void copy_libs();

  /**
   * @param archive
   * @param expected
   */
  void check_pkg_archive(const std::filesystem::path &archive, const std::string &expected);
};

#endif //_REGISTRY_H
