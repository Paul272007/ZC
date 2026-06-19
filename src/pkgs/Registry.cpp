/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include "Registry.h"
#include "../config/PConf.h"
#include "../helpers.h"
#include "../project/Project.h"
#include "Network.h"
#include "pkgs/PkgType.h"
#include <filesystem>

ZC_DEV_CONFIG_JSON

namespace zc
{

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

RegistryPkg Registry::get_pkg(const std::string &name)
{
  const auto it = ranges::find_if(pkgs_, [&](const RegistryPkg &p) { return p.name == name; });
  if (it == pkgs_.end())
    throw ZCException(ZCE_ALREADY_INSTALLED, "The package '" + name + "' was not found");

  return *it;
}

void Registry::install_from_server(
    const std::string &name, const std::string &version, const json &index, const bool force
)
{
  if (!force && is_installed(name))
  {
    if_.info("Skipped package " + name + ": already installed.");
    return;
  }
  const string version_key = version.empty() ? index["packages"][name]["latest"].get<std::string>() : version;

  const fs::path project_root = download_and_extract(name, version, index);
  Project p(project_root);

  finish_install(p, "main");
}

void Registry::install_from_path(const std::filesystem::path &path, const bool force)
{
  Project p(path);
  if (!force && is_installed(p.pconf.name))
  {
    if_.info("Skipped package " + p.pconf.name + ": already installed.");
    return;
  }
  finish_install(p, "local");
}

void Registry::finish_install(Project &p, const std::string &origin)
{
  if_.info("Building package...");
  p.build(BuildMode::release);

  if_.info("Indexing package...");
  index_add_pkg(
      RegistryPkg{
          .name = p.pconf.name,
          .target = p.pconf.target,
          .origin = origin,
          .type = p.pconf.type,
          .versions = {p.pconf.version}
      }
  );
  switch (p.pconf.type)
  {
  case BIN:
    copy_bin(p);
    break;
  case LIB:
    copy_headers(p);
    copy_libs(p);
    break;
  case HEADER:
    copy_headers(p);
    break;
  case COMPOSE:
  default:
    if_.debug("Not implemented yet.");
    break;
  }
}

void Registry::update_from_server(
    const std::string &name, const std::string &version, const nlohmann::json &index, const bool force
)
{
  if (get_pkg(name).origin != "main") // throws an error if package is not installed
    throw ZCException(ZCE_ORIGIN_MISMATCH, "Cannot update local package with distant package");

  // Check if version is already installed
  if (const string version_key =
          version.empty() ? index["packages"][name]["latest"].get<std::string>() : version;
      !force && is_installed(name, version_key))
  {
    if_.info("Skipped package " + name + ": already up-to-date at v" + version_key);
    return;
  }
  const fs::path project_root = download_and_extract(name, version, index);
  Project p(project_root);
  finish_update(p);
}

void Registry::update_from_path(const std::filesystem::path &path, const bool force)
{
  Project p(path);
  if (get_pkg(p.pconf.name).origin != "local") // throws an error if package is not installed
    throw ZCException(ZCE_ORIGIN_MISMATCH, "Cannot update distant package with local package");

  if (!force && is_installed(p.pconf.name, p.pconf.version))
  {
    if_.info("Skipped package " + p.pconf.name + ": already up-to-date at v" + p.pconf.version.string());
    return;
  }
  finish_update(p);
}

void Registry::finish_update(Project &p)
{
  if_.info("Building package...");
  p.build(BuildMode::release);

  if_.info("Indexing package...");
  index_add_pkg_version(p.pconf.name, p.pconf.version);

  switch (p.pconf.type)
  {
  case BIN:
    copy_bin(p);
    break;
  case LIB:
    copy_headers(p);
    copy_libs(p);
    break;
  case HEADER:
    copy_headers(p);
    break;
  case COMPOSE:
  default:
    if_.debug("Not implemented yet.");
    break;
  }
}

void Registry::uninstall(const std::string &pkg)
{
  const RegistryPkg p = unindex_pkg(pkg); // throws error if not found

  if (p.type == BIN)
    if (const auto target = bin_links_dir_ / p.target; fs::exists(target))
      fs::remove(target);

  // Remove entire directory
  if (const auto pkg_path = cache_dir_ / p.name; fs::exists(p.name))
    fs::remove_all(pkg_path);
}

bool Registry::is_installed(const string &name, const Version &version)
{
  const auto it = ranges::find_if(
      pkgs_,
      [&](const RegistryPkg &p)
      {
        return p.name == name &&
               ranges::find_if(p.versions, [&](const Version &v) { return v == version; }) !=
                   p.versions.end();
      }
  );
  return it != pkgs_.end();
}

bool Registry::is_installed(const string &name)
{
  return ranges::find_if(pkgs_, [&](const RegistryPkg &p) { return p.name == name; }) != pkgs_.end();
}

std::vector<RegistryPkg> Registry::pkgs() const
{
  return pkgs_;
}

Table Registry::pkgs_table() const
{
  vector<vector<string>> str_pkgs{{"Package name", "Target", "Origin", "Latest version", "Type"}};

  for (const auto &p : pkgs_)
    str_pkgs.push_back({p.name, p.target, p.origin, p.versions.back().string(), pkg_type_to_str(p.type)});

  return {false, true, str_pkgs};
}

std::vector<std::string> Registry::remote_pkgs() const
{
  const Network &n(Network::get());
  json index = n.get_index();
  vector<std::string> v;
  if (index.contains("packages") && index["packages"].is_object())
    for (auto it = index["packages"].begin(); it != index["packages"].end(); ++it) v.push_back(it.key());
  clean();
  return v;
}

Table Registry::remote_pkgs_table() const
{
  vector<vector<string>> str_pkgs{{"Package name"}};
  for (const auto &p : remote_pkgs()) str_pkgs.push_back({p});
  return {false, true, str_pkgs};
}

Registry::~Registry()
{
  if (modified_)
    Registry::write();
}

void Registry::load()
{
  const json root = if_.read_json(file_);

  get_key(root, "packages", pkgs_);
}

void Registry::write()
{
  const json root{{"packages", pkgs_}};
  if_.write_json(root, file_);
}

Registry::Registry(const std::filesystem::path &root)
    : Conf(root / REGISTRY_FILE), cache_dir_(root / ZC_CACHE_DIR), bin_dir_(cache_dir_ / BIN_DIR),
      lib_dir_(cache_dir_ / LIB_DIR), include_dir_(cache_dir_ / INCLUDE_DIR), bin_links_dir_(root / BIN_DIR),
      tmp_dir_(root / TMP_DIR), net_(Network::get())
{
  if (!fs::exists(file_))
    throw ZCException(ZCE_NOT_FOUND, "Registry file was not found: " + file_.string());
  Registry::load();
}

void Registry::index_add_pkg(const RegistryPkg &pkg)
{
  // Check if already installed
  if (is_installed(pkg.name))
    throw ZCException(ZCE_ALREADY_INSTALLED, "Package " + pkg.name + " is already installed.");

  pkgs_.push_back(pkg);
  modified_ = true;
}

void Registry::index_add_pkg_version(const std::string &name, const Version &version)
{
  const auto pkg = get_pkg_it(name); // throws error if not found

  pkg->versions.push_back(version);
  modified_ = true;
}

std::vector<RegistryPkg>::iterator Registry::get_pkg_it(const std::string &name)
{
  const auto it = ranges::find_if(pkgs_, [&](const RegistryPkg &p) { return p.name == name; });
  if (it == pkgs_.end())
    throw ZCException(ZCE_ALREADY_INSTALLED, "The package '" + name + "' was not found");

  return it;
}

RegistryPkg Registry::unindex_pkg(const string &name)
{
  const auto removed_pkg = get_pkg_it(name);
  pkgs_.erase(removed_pkg);
  return *removed_pkg;
}

void Registry::copy_bin(const Project &p) const
{
  if_.info("Installing binary...");

  const auto source = p.build_dir / p.pconf.target;
  const auto dest_dir = cache_dir_ / p.pconf.name / p.pconf.version.string() / BIN_DIR;
  const auto dest = dest_dir / p.pconf.target;

  if (!fs::exists(source))
    throw ZCException(ZCE_NOT_FOUND, "The compiled binary was not found : " + source.string());

  fs::create_directories(dest_dir);
  fs::copy_file(source, dest, fs::copy_options::overwrite_existing);
  fs::permissions(
      dest, fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec, fs::perm_options::add
  );
}

void Registry::copy_headers(const Project &p) const
{
  if_.info("Installing header(s)...");

  const auto source_dir = p.root_dir / INCLUDE_DIR; // FIX : only headers from include/ are moved
  const auto dest_dir = cache_dir_ / p.pconf.name / p.pconf.version.string() / INCLUDE_DIR;

  if (!fs::exists(source_dir))
    throw ZCException(
        ZCE_NOT_FOUND, "The include directory of the package was not found : " + source_dir.string()
    );

  fs::create_directories(dest_dir);
  fs::copy(source_dir, dest_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
}

void Registry::copy_libs(const Project &p) const
{
  if_.info("Installing libraries...");

  const auto source_dir = p.root_dir / BUILD_DIR;
  const auto dest_dir = cache_dir_ / p.pconf.name / p.pconf.version.string() / LIB_DIR;

  if (!fs::exists(source_dir))
    throw ZCException(ZCE_NOT_FOUND, "The package build directory was not found : " + source_dir.string());

  fs::create_directories(dest_dir);
  const auto pconf = p.pconf;
  fs::copy_file(
      source_dir / STATIC_LIB_NAME, dest_dir / STATIC_LIB_NAME, fs::copy_options::overwrite_existing
  );
  fs::copy_file(
      source_dir / SHARED_LIB_NAME, dest_dir / SHARED_LIB_NAME, fs::copy_options::overwrite_existing
  );
}

void Registry::clean() const
{
  fs::remove_all(tmp_dir_);
}

std::filesystem::path
Registry::download_and_extract(const std::string &name, const std::string &version, const json &index) const
{
  if_.info("Downloading archive...");
  const string archive_url = pkg_url(name, version, index); // throws error if pkg not found in index
  const fs::path archive_path = tmp_dir_ / (name + ".tar.gz");
  const fs::path extract_path = tmp_dir_; // extract directly in ~/.zc/tmp
  net_.download(archive_url, archive_path);

  // TODO : optimise with get_key()
  if_.info("Verifying archive hash...");
  verify_archive_hash(archive_path, index["packages"][name]["versions"][version]["sha256"]);

  if_.info("Extracting archive...");
  fs::create_directories(extract_path);
  extract(archive_path, extract_path); // throws error if archive cannot be extracted

  if_.info("Installing package...");
  fs::path project_root;
  for (const auto &entry : fs::directory_iterator(extract_path)) // get project root
    if (entry.is_directory())
    {
      project_root = entry.path();
      break;
    }
  return project_root;
}

void Registry::verify_archive_hash(const std::filesystem::path &archive, const string &expected) const
{
  if (expected == "SKIP")
    return;

  if (const string actual = sha256(archive); actual != expected)
  {
    fs::remove(archive);
    throw ZCException(
        ZCE_HASH_MISMATCH, "SECURITY ALERT: Hash mismatch!\nExpected: " + expected + "\nActual:   " + actual
    );
  }
  if_.success("Hash is correct");
}

std::string Registry::pkg_url(const string &name, const string &version, const nlohmann::json &index)
{
  // TODO : optimise with get_key()
  if (!index.contains("packages"))
    throw ZCException(ZCE_MISSING_PROPERTY, "Index should contain hey 'packages'.");

  if (!index["packages"].contains(name))
    throw ZCException(ZCE_PKG_NOT_FOUND, "Package '" + name + "' not found.");

  const string version_to_install =
      version.empty() ? index["packages"][name]["latest"].get<std::string>() : version;

  if (!index["packages"][name]["versions"].contains(version_to_install))
    throw ZCException(ZCE_NOT_FOUND, "Version " + version_to_install + " does not exist.");

  return index["packages"][name]["versions"][version_to_install].value("url", "");
}

} // namespace zc
