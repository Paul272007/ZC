/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include "Registry.h"
#include "../helpers.h"
#include "Network.h"

ZC_DEV_CONFIG

/**
 * Registry implementation
 */

/**
 * Get the user registry : ~/.zc/registry.json
 * @return Registry
 */
Registry &Registry::get()
{
  static Registry instance(get_zc_root());
  return instance;
}

/**
 * Uninstall a package
 * @param pkg
 * @return bool
 */
bool Registry::remove_pkg(string pkg)
{
  return false;
}

/**
 * @param name
 * @return Pkg
 */
Pkg Registry::get_pkg(string name)
{
  return;
}

/**
 * @return std::vector<Pkg>
 */
std::vector<Pkg> Registry::pkgs()
{
  return pkgs_;
}

/**
 * @param name
 * @param version
 * @return void
 */
void Registry::install_pkg(string name, Version version)
{
  return;
}

/**
 * @param name
 * @return void
 */
void Registry::update_pkg(string name)
{
  return;
}

/**
 * @return void
 */
void Registry::load()
{
  return;
}

/**
 * @return void
 */
void Registry::write()
{
  return;
}

/**
 * @param root
 */
Registry::Registry(std::filesystem::path root) : net_(Network::get())
{
}

/**
 * @param pkg
 * @return void
 */
void Registry::index_pkg(Pkg pkg)
{
  return;
}

/**
 * Remove pkg from index. A package with the same name and version must be found in the registry index.
 * @param pkg
 * @return void
 */
void Registry::unindex_pkg(Pkg pkg)
{
  return;
}

/**
 * Check if pkg is installed
 * @param pkg
 * @return bool
 */
bool Registry::is_installed(string pkg)
{
  return false;
}

/**
 * @return void
 */
void Registry::copy_bin()
{
  return;
}

/**
 * @return void
 */
void Registry::copy_libs()
{
  return;
}

/**
 * @param archive
 * @param expected
 * @return void
 */
void Registry::check_pkg_archive(std::filesystem::path archive, string expected)
{
  return;
}
