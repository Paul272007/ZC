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
  bool remove_pkg(std::string pkg);

  /**
   * @param name
   */
  Pkg get_pkg(std::string name);

  std::vector<Pkg> pkgs();

  /**
   * @param name
   * @param version
   */
  void install_pkg(std::string name, Version version);

  /**
   * @param name
   */
  void update_pkg(std::string name);

protected:
  void load();

  void write();

private:
  const std::filesystem::path root_dir_;
  const std::filesystem::path bin_dir_;
  const std::filesystem::path lib_dir_;
  const std::filesystem::path include_dir_;
  static const std::filesystem::path tmp_dir_;
  std::vector<Pkg> pkgs_;
  Network &net_;

  /**
   * @param root
   */
  Registry(std::filesystem::path root);

  /**
   * @param pkg
   */
  void index_pkg(Pkg pkg);

  /**
   * Remove pkg from index. A package with the same name and version must be found in the registry index.
   * @param pkg
   */
  void unindex_pkg(Pkg pkg);

  /**
   * Check if pkg is installed
   * @param pkg
   */
  bool is_installed(std::string pkg);

  void copy_bin();

  void copy_libs();

  /**
   * @param archive
   * @param expected
   */
  void check_pkg_archive(std::filesystem::path archive, std::string expected);
};

#endif //_REGISTRY_H
