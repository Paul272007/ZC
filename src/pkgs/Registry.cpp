#include "Registry.h"

#include <algorithm>
#include <filesystem>
#include <vector>

#include "config/PConf.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "Network.h"
#include "PkgType.h"
#include "project/Project.h"
#include "RemoteTarget.h"
#include "ui/Interface.h"
#include "Version.h"

ZC_DEV_CONFIG_JSON

namespace zc
{

Registry &Registry::get()
{
  static Registry instance(zc_root());
  return instance;
}

Pkg Registry::get_pkg(const std::string &name)
{
  auto it = pkgs_.find(name);
  if (it == pkgs_.end())
    throw ZCException(ZCE_PKG_NOT_FOUND, "Package '" + name + "' was not found");
  return it->second;
}

Dependency Registry::get_dependency(const LocalTarget &t)
{
  auto it = pkgs_.find(t.name);

  if (it == pkgs_.end())
    throw ZCException(ZCE_PKG_NOT_FOUND, "Package '" + t.name + "' was not found");
  if (it->second.type == PkgType::BIN)
    throw ZCException(ZCE_TYPE_ERROR, "Cannot add dependency of type BIN");

  const std::vector<Version> &versions{ it->second.versions };

  Version version_to_use{ t.version };

  if (version_to_use.is_empty() || version_to_use.is_default())
    version_to_use = it->second.default_version;
  else if (version_to_use.is_latest())
    version_to_use = *ranges::max_element(versions);
  else if (const auto version_it = ranges::find(versions, t.version); version_it == versions.end())
    throw ZCException(
      ZCE_PKG_NOT_FOUND, "Package '" + t.name + "' at version " + t.version.string() + " was not found"
    );

  return {
    .name    = it->second.name,
    .origin  = it->second.origin,
    .version = version_to_use,
  };
}

void Registry::install_std(const std::string &name)
{
  std::string target = name;

  if (name == "math")
    target = "m";
  elif (name == "opengl")
    target = "GL";

  if (get_pkg_config_flags(name, false).empty())
    ui().warning("Package '" + name + "' not found by pkg-config. Assuming it's a built-in OS library.");
  else
    ui().success("System package found: " + name);

  add_pkg_to_index(
    {
      .name            = name,
      .target          = target,
      .origin          = "std",
      .type            = PkgType::LIB,
      .default_version = { 0, 0, 0 },
      .versions        = { 0, 0, 0 },
    }
  );
}

void Registry::install_from_server(const RemoteTarget &target, const bool force)
{
  if (!force && is_installed(target.name))
  {
    update_from_server(target, force, true);
    return;
  }
  Project p(download_and_extract(target));
  finish_install(p, "main");
}

void Registry::install_from_path(const std::filesystem::path &path, const bool force)
{
  Project p(path);
  if (!force && is_installed(p.pconf.name))
  {
    update_from_path(path, force, true);
    return;
  }
  finish_install(p, "local");
}

void Registry::finish_install(Project &p, const std::string &origin)
{
  ui().debug("Building package...");
  verify_headers_structure(p);
  p.install_dependencies();
  p.build(BuildMode::release, true);

  ui().debug("Indexing package...");
  Pkg pkg{
    .name            = p.pconf.name,
    .target          = p.pconf.target,
    .origin          = origin,
    .type            = p.pconf.type,
    .default_version = p.pconf.version,
    .versions        = { p.pconf.version },
  };
  add_pkg_to_index(pkg);
  switch (p.pconf.type)
  {
  case PkgType::BIN:
    copy_bin(p);
    break;
  case PkgType::LIB:
    copy_headers(p);
    copy_libs(p);
    break;
  case PkgType::HEADER:
    copy_headers(p);
    break;
  case PkgType::COMPOSE:
  default:
    ui().debug("Not implemented yet.");
    break;
  }
  update_symlinks(pkg);
  ui().success("Package successfully installed!");
}

void Registry::update_from_server(const RemoteTarget &target, const bool force, const bool use)
{
  if (get_pkg(target.name).origin != "main") // throws an error if package is not installed
    throw ZCException(ZCE_ORIGIN_MISMATCH, "Cannot update local package with distant package");

  if (!force && is_installed(target.name, target.version))
  {
    ui().info("Skipped package " + target.name + ": already up-to-date at v" + target.version.string());
    return;
  }
  Project p(download_and_extract(target));
  finish_update(p, use);
}

Project Registry::update_from_path(const std::filesystem::path &path, const bool force, const bool use)
{
  Project p(path);
  if (get_pkg(p.pconf.name).origin != "local") // throws an error if package is not installed
    throw ZCException(ZCE_ORIGIN_MISMATCH, "Cannot update distant package with local package");

  if (!force && is_installed(p.pconf.name, p.pconf.version))
  {
    ui().info("Skipped package " + p.pconf.name + ": already up-to-date at v" + p.pconf.version.string());
    return p;
  }
  finish_update(p, use);
  return p;
}

void Registry::finish_update(Project &p, const bool use)
{
  ui().debug("Building package...");
  verify_headers_structure(p);
  p.install_dependencies();
  p.build(BuildMode::release, true);

  ui().debug("Indexing package...");
  add_version_to_pkg(p.pconf.name, p.pconf.version);

  switch (p.pconf.type)
  {
  case PkgType::BIN:
    copy_bin(p);
    break;
  case PkgType::LIB:
    copy_headers(p);
    copy_libs(p);
    break;
  case PkgType::HEADER:
    copy_headers(p);
    break;
  case PkgType::COMPOSE:
  default:
    ui().debug("Not implemented yet.");
    break;
  }
  if (use)
    set_default_version(p.pconf.name, p.pconf.version);
  ui().success("Package '" + p.pconf.name + "' updated successfully");
}

void Registry::uninstall(const std::string &pkg)
{
  const Pkg p = remove_pkg_from_index(pkg); // throws error if not found

  if (p.type == PkgType::BIN)
    if (const auto target = bin_links_dir_ / p.target; fs::exists(target))
      fs::remove(target);

  // Remove entire directory
  if (const auto pkg_path = cache_dir_ / p.name; fs::exists(pkg_path))
    fs::remove_all(pkg_path);
}

Version Registry::get_latest(const std::string &name)
{
  const Pkg &pkg = get_pkg(name);
  return *ranges::max_element(pkg.versions);
}

bool Registry::is_installed(const std::string &name, const Version &v)
{
  const auto it = pkgs_.find(name);
  if (it == pkgs_.end())
    return false;

  const std::vector<Version> &versions = it->second.versions;
  return ranges::find(versions, v) != versions.end();
}

bool Registry::is_installed(const string &name) const
{
  return pkgs_.contains(name);
}

std::map<std::string, Pkg> Registry::pkgs() const
{
  return pkgs_;
}

Table Registry::pkgs_table() const
{
  vector<vector<string>> str_pkgs{ { "Package name", "Target", "Origin", "Type", "Default version" } };

  for (const auto &[name, target, origin, type, default_version, versions] : pkgs_ | views::values)
    str_pkgs.push_back({ name, target, origin, pkg_type_to_pretty_str(type), default_version.string() });

  return { false, true, str_pkgs };
}

std::vector<std::pair<std::string, std::string>> Registry::remote_pkgs() const
{
  json index = net().get_index();

  vector<pair<string, string>> v;
  if (index.contains("packages") && index["packages"].is_object())
    for (auto it = index["packages"].begin(); it != index["packages"].end(); ++it)
      v.emplace_back(it.key(), it.value().contains("latest") ? it.value()["latest"] : "");
  clean();
  return v;
}

Table Registry::remote_pkgs_table() const
{
  vector<vector<string>> str_pkgs{ { "Package name", "Latest version" } };
  for (const auto &[fst, snd] : remote_pkgs())
    str_pkgs.push_back({ fst, snd });
  return { false, true, str_pkgs };
}

Registry::~Registry()
{
  clean();
  if (modified_)
    Registry::write();
}

void Registry::load()
{
  if (const json root = read_json(file_); root.contains("packages") && root["packages"].is_object())
  {
    pkgs_.clear();
    for (CAA[key, value] : root["packages"].items())
    {
      auto pkg = value.get<Pkg>();
      pkg.name = key;
      pkgs_.insert_or_assign(key, pkg);
    }
  }
}

void Registry::write()
{
  json root;

  json pkgs_json = json::object();
  for (CAA[name, conf] : pkgs_)
    pkgs_json[name] = conf;
  root["packages"] = pkgs_json;

  write_json(root, file_);
}

Registry::Registry(const std::filesystem::path &root)
  : Conf(root / REGISTRY_FILE),
    tmp_dir_(root / TMP_DIR),
    cache_dir_(root / ZC_CACHE_DIR),
    bin_links_dir_(root / BIN_DIR),
    lib_links_dir_(root / LIB_DIR),
    include_links_dir_(root / INCLUDE_DIR)
{
  if (!fs::exists(file_))
    throw ZCException(ZCE_NOT_FOUND, "Registry file was not found: " + file_.string());
  Registry::load();
}

void Registry::add_pkg_to_index(const Pkg &pkg)
{
  pkgs_.insert_or_assign(pkg.name, pkg);
  modified_ = true;
}

void Registry::add_version_to_pkg(const std::string &name, const Version &version)
{
  const auto pkg = get_pkg_it(name); // throws error if not found
  if (auto &versions = pkg->second.versions; ranges::find(versions, version) == versions.end())
    versions.push_back(version);
  modified_ = true;
}

void Registry::set_default_version(const std::string &name, const Version &version)
{
  auto it = get_pkg_it(name);
  if (auto &versions = it->second.versions; ranges::find(versions, version) == versions.end())
    throw ZCException(
      ZCE_PKG_NOT_FOUND, "Version " + version.string() + " is not installed for package " + name
    );

  it->second.default_version = version;
  modified_                  = true;
  update_symlinks(it->second);
}

std::map<std::string, Pkg>::iterator Registry::get_pkg_it(const std::string &name)
{
  auto it = pkgs_.find(name);
  if (it == pkgs_.end())
    throw ZCException(ZCE_PKG_NOT_FOUND, "Package '" + name + "' was not found");
  return it;
}

Pkg Registry::remove_pkg_from_index(const std::string &name)
{
  const auto it            = get_pkg_it(name);
  Pkg        extracted_pkg = std::move(it->second);
  pkgs_.erase(it);
  modified_ = true;
  return extracted_pkg;
}

void Registry::copy_bin(const Project &p) const
{
  ui().info("Installing binary...");

  const auto source   = p.build_dir / p.pconf.target;
  const auto dest_dir = cache_dir_ / p.pconf.name / p.pconf.version.string() / BIN_DIR;
  const auto dest     = dest_dir / p.pconf.target;

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
  ui().info("Installing header(s)...");

  const auto source_dir = p.root_dir / INCLUDE_DIR / p.pconf.name;
  const auto dest_dir   = cache_dir_ / p.pconf.name / p.pconf.version.string() / INCLUDE_DIR / p.pconf.name;

  if (!fs::exists(source_dir))
    throw ZCException(
      ZCE_NOT_FOUND, "The include directory of the package was not found : " + source_dir.string()
    );

  fs::create_directories(dest_dir);
  fs::copy(source_dir, dest_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
}

void Registry::copy_libs(const Project &p) const
{
  ui().info("Installing libraries...");

  const auto source_dir = p.root_dir / BUILD_DIR;
  const auto dest_dir   = cache_dir_ / p.pconf.name / p.pconf.version.string() / LIB_DIR;

  if (!fs::exists(source_dir))
    throw ZCException(ZCE_NOT_FOUND, "The package build directory was not found : " + source_dir.string());

  fs::create_directories(dest_dir);
  fs::copy_file(
    source_dir / STATIC_LIB_NAME(p.pconf.target), dest_dir / STATIC_LIB_NAME(p.pconf.target),
    fs::copy_options::overwrite_existing
  );
  fs::copy_file(
    source_dir / SHARED_LIB_NAME(p.pconf.target), dest_dir / SHARED_LIB_NAME(p.pconf.target),
    fs::copy_options::overwrite_existing
  );
}

void Registry::update_symlinks(const Pkg &p) const
{
  std::string ver       = p.default_version.string();
  const auto  pkg_cache = cache_dir_ / p.name / ver;

  if (p.type == PkgType::BIN)
  {
    const auto dest = pkg_cache / BIN_DIR / p.target;
    const auto link = bin_links_dir_ / p.target;
    fs::create_directories(bin_links_dir_);
    if (fs::exists(link) || fs::is_symlink(link))
      fs::remove(link);
    fs::create_symlink(dest, link);
  }
  else if (p.type == PkgType::LIB || p.type == PkgType::HEADER)
  {
    const auto dest_include = pkg_cache / INCLUDE_DIR / p.name;
    const auto link_include = include_links_dir_ / p.name;
    fs::create_directories(include_links_dir_);
    if (fs::exists(link_include) || fs::is_symlink(link_include))
    {
      if (fs::is_directory(link_include))
        fs::remove_all(link_include);
      else
        fs::remove(link_include);
    }
    fs::create_directory_symlink(dest_include, link_include);

    if (p.type == PkgType::LIB)
    {
      const auto dest_lib = pkg_cache / LIB_DIR;
      const auto link_lib = lib_links_dir_ / p.name;
      fs::create_directories(lib_links_dir_);
      if (fs::exists(link_lib) || fs::is_symlink(link_lib))
      {
        if (fs::is_directory(link_lib))
          fs::remove_all(link_lib);
        else
          fs::remove(link_lib);
      }
      fs::create_directory_symlink(dest_lib, link_lib);
    }
  }
}

void Registry::clean() const
{
  if (fs::exists(tmp_dir_) && fs::is_directory(tmp_dir_))
    fs::remove_all(tmp_dir_);
}

std::filesystem::path Registry::download_and_extract(const RemoteTarget &target) const
{
  ui().info("Downloading archive...");
  const string   archive_url  = target.url;
  const fs::path archive_path = tmp_dir_ / (target.name + ".tar.gz");
  const fs::path extract_path = tmp_dir_;
  net().download(archive_url, archive_path);

  ui().debug("Verifying archive hash...");
  verify_archive_hash(archive_path, target.sha256);

  ui().debug("Extracting archive...");
  fs::create_directories(extract_path);
  extract(archive_path, extract_path);
  fs::path project_root;
  for (const auto &entry : fs::directory_iterator(extract_path))
    if (entry.is_directory())
    {
      project_root = entry.path();
      break;
    }
  return project_root;
}

void Registry::verify_archive_hash(const std::filesystem::path &archive, const string &expected)
{
  if (expected == "SKIP")
    return;

  if (const string actual = sha256(archive); actual != expected)
  {
    fs::remove(archive);
    throw ZCException(
      ZCE_HASH_MISMATCH, "SECURITY ALERT: Hash mismatch! Expected: " + expected + ", Received:   " + actual
    );
  }
  ui().success("Hash is correct");
}

void Registry::verify_headers_structure(const Project &p)
{
  if (p.pconf.type == PkgType::LIB || p.pconf.type == PkgType::HEADER)
    if (!fs::exists(p.root_dir / INCLUDE_DIR / p.pconf.name))
      throw ZCException(
        ZCE_BAD_STRUCTURE,
        "Public headers must be inside 'include/" + p.pconf.name + "/' to avoid collisions."
      );
}

} // namespace zc
