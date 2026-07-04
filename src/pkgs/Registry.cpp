#include "Registry.h"

#include <filesystem>
#include <ranges>
#include <string>
#include <vector>

#include "config/PConf.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "Network.h"
#include "pkgs/Pkg.h"
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

const Pkg &Registry::get_pkg(const std::string &name)
{
  auto it = pkgs_.find(name);
  if (it == pkgs_.end())
    throw ZCException(ZCE_PKG_NOT_FOUND, "Package '" + name + "' was not found");
  return it->second;
}

void Registry::install_std(const std::string &name, const bool force)
{
  if (!force && is_installed(name))
  {
    ui().info("Skipped package " + name + ": already installed.");
    return;
  }
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
      .path            = "",
      .name            = name,
      .target          = target,
      .origin          = "std",
      .type            = PkgType::LIB,
      .default_version = Version::empty(),
      .versions        = { { Version::empty(), {} } },
    }
  );
}

void Registry::install_from_server(const RemoteTarget &target, const bool force, const size_t jobs)
{
  if (!force && is_installed(target.name))
  {
    update_from_server(target, force, true);
    return;
  }
  Project p(download_and_extract(target));
  finish_install(p, "main", jobs);
}

Project Registry::install_from_path(
  const std::filesystem::path &path, const bool force, const bool save_path, const size_t jobs
)
{
  Project p(get_project_root(path));
  if (!force && is_installed(p.pconf.name))
    return update_from_path(path, force, true, save_path, jobs);

  finish_install(p, "local", jobs);
  if (save_path)
    set_path(p.pconf.name, p.root_dir);
  return p;
}

void Registry::finish_install(Project &p, const std::string &origin, const size_t jobs)
{
  ui().debug("Building package...");
  verify_headers_structure(p);
  p.install_dependencies(false);
  p.build(BuildMode::release, true, jobs);

  ui().debug("Indexing package...");

  // Get dependencies :
  std::map<std::string, Version> deps;
  for (const auto &[dep_name, dep] : p.pconf.dependencies)
    if (!dep.static_link)
      deps[dep_name] = dep.version;

  Pkg pkg{
    .path            = "",
    .name            = p.pconf.name,
    .target          = p.pconf.target,
    .origin          = origin,
    .type            = p.pconf.type,
    .default_version = p.pconf.version,
    .versions        = { { p.pconf.version, deps } },
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

void Registry::update_from_server(
  const RemoteTarget &target, const bool force, const bool use, const size_t jobs
)
{
  if (get_pkg(target.name).origin != "main") // throws an error if package is not installed
    throw ZCException(ZCE_ORIGIN_MISMATCH, "Cannot update local package with distant package");

  if (!force && is_installed(target.name, target.version))
  {
    ui().info("Skipped package " + target.name + ": already up-to-date at v" + target.version.string());
    return;
  }
  Project p(download_and_extract(target));
  finish_update(p, use, jobs);
}

Project Registry::update_from_path(
  const std::filesystem::path &path, const bool force, const bool use, const bool save_path,
  const size_t jobs
)
{
  Project p(get_project_root(path));
  if (get_pkg(p.pconf.name).origin != "local") // throws an error if package is not installed
    throw ZCException(ZCE_ORIGIN_MISMATCH, "Cannot update distant package with local package");

  if (!force && is_installed(p.pconf.name, p.pconf.version))
  {
    ui().info("Skipped package " + p.pconf.name + ": already up-to-date at v" + p.pconf.version.string());
    return p;
  }
  finish_update(p, use, jobs);
  if (save_path)
    set_path(p.pconf.name, p.root_dir);
  return p;
}

void Registry::finish_update(Project &p, const bool use, const size_t jobs)
{
  ui().debug("Building package...");
  verify_headers_structure(p);
  p.install_dependencies(false);
  p.build(BuildMode::release, true, jobs);

  ui().debug("Indexing package...");

  // Get dependencies :
  std::map<std::string, Version> deps;
  for (const auto &[dep_name, dep] : p.pconf.dependencies)
    if (!dep.static_link)
      deps[dep_name] = dep.version;

  add_version_to_pkg(p.pconf.name, p.pconf.version, deps);

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

void Registry::uninstall_from_path(const std::filesystem::path &path, bool force)
{
  Project p(get_project_root(path));
  uninstall(LocalTarget::get_target({ p.pconf.name, p.pconf.version }), force);
}

void Registry::uninstall(const std::string &pkg, const bool force)
{
  auto dependents = dependents_of(pkg);
  if (!dependents.empty())
  {
    ui().warning("The package '" + pkg + "' is required by these packages :");
    for (const auto &[dep_name, dep_version] : dependents)
      ui().warning("  - " + dep_name + "@" + dep_version.string());

    if (!force)
      throw ZCException(ZCE_BROKEN_DEPENDENCY, "Cannot break dependency.");
  }
  const Pkg p = remove_pkg_from_index(pkg); // throws error if not found

  switch (p.type)
  {
  case PkgType::BIN:
    if (const auto target = bin_links_dir_ / p.target; fs::exists(target))
      fs::remove(target);
    break;
  case PkgType::HEADER:
    if (const auto target = include_links_dir_ / p.target; fs::exists(target))
      fs::remove(target);
    break;
  case PkgType::LIB:
    if (const auto target = lib_links_dir_ / p.target; fs::exists(target))
      fs::remove(target);
    if (const auto target = include_links_dir_ / p.target; fs::exists(target))
      fs::remove(target);
    break;
  case PkgType::COMPOSE:
  case PkgType::UNDEF:
  default:
    break;
  }
  if (const auto pkg_path = cache_dir_ / p.name; fs::exists(pkg_path))
    fs::remove_all(pkg_path); // Remove entire package directory
}

void Registry::uninstall(const LocalTarget &t, const bool force)
{
  auto  it       = get_pkg_it(t.name);
  auto &p        = it->second;
  auto &versions = p.versions;
  auto  v_it     = versions.find(t.version);

  if (versions.size() == 1)
  {
    uninstall(t.name, true);
    return;
  }
  auto dependents = dependents_of(t.name, t.version);
  if (!dependents.empty())
  {
    ui().warning("The package '" + t.string() + "' is required by these packages :");
    for (const auto &[dep_name, dep_version] : dependents)
      ui().warning("  - " + dep_name + "@" + dep_version.string());

    if (!force)
      throw ZCException(ZCE_BROKEN_DEPENDENCY, "Cannot break dependency.");
  }

  versions.erase(v_it);
  modified_ = true;

  if (const auto version_path = cache_dir_ / p.name / t.version.string(); fs::exists(version_path))
    fs::remove_all(version_path);

  if (p.default_version == t.version)
  {
    p.default_version = versions.rbegin()->first;
    update_symlinks(p);
    ui().info("Default version of '" + t.name + "' automatically updated to " + p.default_version.string());
  }
}

Version Registry::get_latest(const std::string &name)
{
  const Pkg &pkg = get_pkg(name);
  return pkg.versions.rbegin()->first;
}

bool Registry::is_installed(const std::string &name, const Version &v)
{
  const auto it = pkgs_.find(name);
  if (it == pkgs_.end())
    return false;

  return it->second.versions.contains(v);
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

  for (const auto &[path, name, target, origin, type, default_version, versions] : pkgs_ | views::values)
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

void Registry::add_version_to_pkg(
  const std::string &name, const Version &version, const std::map<std::string, Version> &deps
)
{
  const auto pkg = get_pkg_it(name); // throws error if not found
  if (auto &versions = pkg->second.versions; !versions.contains(version))
    versions.insert_or_assign(version, deps);
  modified_ = true;
}

void Registry::set_default_version(const std::string &name, const Version &version)
{
  auto it = get_pkg_it(name);
  if (!it->second.versions.contains(version))
    throw ZCException(ZCE_PKG_NOT_FOUND, "Version " + version.string() + " not found for package " + name);

  it->second.default_version = version;
  modified_                  = true;
  update_symlinks(it->second);
}

void Registry::set_path(const std::string &name, const std::filesystem::path &path)
{
  auto it         = get_pkg_it(name);
  it->second.path = fs::absolute(path);
  modified_       = true;
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
      fs::remove_all(link_include);
    fs::create_directory_symlink(dest_include, link_include);

    if (p.type == PkgType::LIB)
    {
      const auto dest_lib = pkg_cache / LIB_DIR;
      const auto link_lib = lib_links_dir_ / p.name;
      fs::create_directories(lib_links_dir_);
      if (fs::exists(link_lib) || fs::is_symlink(link_lib))
        fs::remove_all(link_lib);
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

std::vector<std::pair<std::string, Version>>
Registry::dependents_of(const std::string &target_pkg_name) const
{
  std::vector<std::pair<std::string, Version>> dependents;
  for (const auto &[pkg_name, pkg] : pkgs_)
    for (const auto &[version, deps] : pkg.versions)
      if (deps.contains(target_pkg_name))
        dependents.emplace_back(pkg_name, version);
  return dependents;
}

std::vector<std::pair<std::string, Version>>
Registry::dependents_of(const std::string &target_pkg_name, const Version &target_version) const
{
  std::vector<std::pair<std::string, Version>> dependents;
  for (const auto &[pkg_name, pkg] : pkgs_)
    for (const auto &[version, deps] : pkg.versions)
    {
      auto it = deps.find(target_pkg_name);
      if (it != deps.end() && (it->second == target_version || it->second.is_latest() ||
                               it->second.is_empty() || it->second.is_default()))
        dependents.emplace_back(pkg_name, version);
    }
  return dependents;
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
