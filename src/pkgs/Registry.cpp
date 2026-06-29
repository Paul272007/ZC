#include "Registry.h"

#include <algorithm>
#include <filesystem>
#include <vector>

#include "../config/PConf.h"
#include "../helpers.h"
#include "../project/Project.h"
#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "Network.h"
#include "PkgType.h"
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

Dependency Registry::get_dependency(const Target &target)
{
  auto it = pkgs_.find(target.name);

  if (it == pkgs_.end())
    throw ZCException(ZCE_PKG_NOT_FOUND, "Package '" + target.name + "' was not found");
  if (it->second.type == PkgType::BIN)
    throw ZCException(ZCE_TYPE_ERROR, "Cannot add dependency of type BIN");

  const std::vector<Version> &versions{ it->second.versions };

  Version version_to_use{ target.version };

  if (!version_to_use.empty()) // if version was precised verify that it exists
  {
    if (const auto version_it = ranges::find(versions, target.version); version_it == versions.end())
      throw ZCException(
        ZCE_PKG_NOT_FOUND,
        "Package '" + target.name + "' at version " + target.version.string() + " was not found"
      );
  }
  else // else find the latest version
    version_to_use = *ranges::max_element(versions);

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
    if_.warning(
      "Package '" + name + "' not found by pkg-config. Assuming it's a built-in OS library (-l" + target +
      ")."
    );
  else
    if_.success("System package found: " + name);

  index_add_pkg(
    {
      .name     = name,
      .target   = target,
      .origin   = "std",
      .type     = PkgType::LIB,
      .versions = { Version::latest() },
    }
  );
}

void Registry::install_from_server(Target &target, const json &index, const bool force)
{
  if (is_installed(target.name))
  {
    update_from_server(target, index, force);
    return;
  }

  target.version =
    target.version.empty() ? index["packages"][target.name]["latest"].get<string>() : target.version;

  Project p(download_and_extract(target, index));

  finish_install(p, "main");
}

void Registry::install_from_path(const std::filesystem::path &path, const bool force)
{
  Project p(path);
  if (is_installed(p.pconf.name))
  {
    update_from_path(path, force);
    return;
  }
  finish_install(p, "local");
}

void Registry::finish_install(Project &p, const std::string &origin)
{
  if_.info("Building package...");
  verify_headers_structure(p);
  p.install_dependencies();
  p.build(BuildMode::release, true);

  if_.info("Indexing package...");
  index_add_pkg(
    { .name     = p.pconf.name,
      .target   = p.pconf.target,
      .origin   = origin,
      .type     = p.pconf.type,
      .versions = { p.pconf.version } }
  );
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
    if_.debug("Not implemented yet.");
    break;
  }
  if_.success("Package successfully installed!");
}

void Registry::update_from_server(Target &target, const nlohmann::json &index, const bool force)
{
  if (get_pkg(target.name).origin != "main") // throws an error if package is not installed
    throw ZCException(ZCE_ORIGIN_MISMATCH, "Cannot update local package with distant package");

  // Check if version is already installed
  target.version =
    target.version.empty() ? index["packages"][target.name]["latest"].get<string>() : target.version;
  if (!force && is_installed(target))
  {
    if_.info("Skipped package " + target.name + ": already up-to-date at v" + target.version.string());
    return;
  }
  Project p(download_and_extract(target, index));
  finish_update(p);
}

Project Registry::update_from_path(const std::filesystem::path &path, const bool force)
{
  Project p(path);
  if (get_pkg(p.pconf.name).origin != "local") // throws an error if package is not installed
    throw ZCException(ZCE_ORIGIN_MISMATCH, "Cannot update distant package with local package");

  if (!force && is_installed({ .name = p.pconf.name, .version = p.pconf.version }))
  {
    if_.info("Skipped package " + p.pconf.name + ": already up-to-date at v" + p.pconf.version.string());
    return p;
  }
  finish_update(p);
  return p;
}

void Registry::finish_update(Project &p)
{
  if_.info("Building package...");
  verify_headers_structure(p);
  p.install_dependencies();
  p.build(BuildMode::release, true);

  if_.info("Indexing package...");
  index_add_pkg_version(p.pconf.name, p.pconf.version);

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
    if_.debug("Not implemented yet.");
    break;
  }
}

void Registry::uninstall(const std::string &pkg)
{
  const Pkg p = unindex_pkg(pkg); // throws error if not found

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

bool Registry::is_installed(const Target &target)
{
  const auto it = pkgs_.find(target.name);
  if (it == pkgs_.end())
    return false;

  const std::vector<Version> &versions = it->second.versions;
  return ranges::find(versions, target.version) != versions.end();
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
  vector<vector<string>> str_pkgs{ { "Package name", "Target", "Origin", "Latest version", "Type" } };

  for (const auto &[name, target, origin, type, versions] : pkgs_ | views::values)
    str_pkgs.push_back(
      {
        name,
        target,
        origin,
        versions.back().string(),
        pkg_type_to_pretty_str(type),
      }
    );

  return { false, true, str_pkgs };
}

std::vector<std::pair<std::string, std::string>> Registry::remote_pkgs() const
{
  json index = net_.get_index();

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
    cache_dir_(root / ZC_CACHE_DIR),
    tmp_dir_(root / TMP_DIR),
    include_links_dir_(root / INCLUDE_DIR),
    lib_links_dir_(root / LIB_DIR),
    bin_links_dir_(root / BIN_DIR)
{
  if (!fs::exists(file_))
    throw ZCException(ZCE_NOT_FOUND, "Registry file was not found: " + file_.string());
  Registry::load();
}

void Registry::index_add_pkg(const Pkg &pkg)
{
  if (is_installed(pkg.name))
    throw ZCException(ZCE_ALREADY_INSTALLED, "Package " + pkg.name + " is already installed.");

  pkgs_.insert_or_assign(pkg.name, pkg);
  modified_ = true;
}

void Registry::index_add_pkg_version(const std::string &name, const Version &version)
{
  const auto pkg = get_pkg_it(name); // throws error if not found

  if (auto &versions = pkg->second.versions; ranges::find(versions, version) == versions.end())
    versions.push_back(version);
  modified_ = true;
}

std::map<std::string, Pkg>::iterator Registry::get_pkg_it(const std::string &name)
{
  auto it = pkgs_.find(name);
  if (it == pkgs_.end())
    throw ZCException(ZCE_PKG_NOT_FOUND, "Package '" + name + "' was not found");
  return it;
}

Pkg Registry::unindex_pkg(const std::string &name)
{
  const auto it            = get_pkg_it(name);
  Pkg        extracted_pkg = std::move(it->second);
  pkgs_.erase(it);
  modified_ = true;
  return extracted_pkg;
}

void Registry::copy_bin(const Project &p) const
{
  if_.info("Installing binary...");

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
  // Edit symlink in ~/.zc/bin
  const auto link = bin_links_dir_ / p.pconf.target;
  fs::create_directories(bin_links_dir_);
  if (fs::exists(link) || fs::is_symlink(link))
    fs::remove(link);
  fs::create_symlink(dest, link);
}

void Registry::copy_headers(const Project &p) const
{
  if_.info("Installing header(s)...");

  const auto source_dir = p.root_dir / INCLUDE_DIR / p.pconf.name;
  const auto dest_dir   = cache_dir_ / p.pconf.name / p.pconf.version.string() / INCLUDE_DIR / p.pconf.name;

  if (!fs::exists(source_dir))
    throw ZCException(
      ZCE_NOT_FOUND, "The include directory of the package was not found : " + source_dir.string()
    );

  fs::create_directories(dest_dir);
  fs::copy(source_dir, dest_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
  // Edit symlink in ~/.zc/include for zc run
  const auto link = include_links_dir_ / p.pconf.name;
  fs::create_directories(include_links_dir_);
  if (fs::exists(link) || fs::is_symlink(link))
  {
    if (fs::is_directory(link))
      fs::remove_all(link);
    else
      fs::remove(link);
  }
  fs::create_directory_symlink(dest_dir, link);
}

void Registry::copy_libs(const Project &p) const
{
  if_.info("Installing libraries...");

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
  // Edit symlink in ~/.zc/lib for zc run
  const auto link = lib_links_dir_ / p.pconf.name;
  fs::create_directories(lib_links_dir_);
  if (fs::exists(link) || fs::is_symlink(link))
  {
    if (fs::is_directory(link))
      fs::remove_all(link);
    else
      fs::remove(link);
  }
  fs::create_directory_symlink(dest_dir, link);
}

void Registry::clean() const
{
  if (fs::exists(tmp_dir_) && fs::is_directory(tmp_dir_))
    fs::remove_all(tmp_dir_);
}

std::filesystem::path Registry::download_and_extract(const Target &target, const json &index) const
{
  if_.info("Downloading archive...");
  const string   archive_url  = pkg_url(target, index); // throws error if pkg not found in index
  const fs::path archive_path = tmp_dir_ / (target.name + ".tar.gz");
  const fs::path extract_path = tmp_dir_;
  const auto    &version_json = index["packages"][target.name]["versions"][target.version.string()];
  net_.download(archive_url, archive_path);

  if_.info("Verifying archive hash...");
  verify_archive_hash(archive_path, version_json.value("sha256", "SKIP"));

  if_.info("Extracting archive...");
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

void Registry::verify_archive_hash(const std::filesystem::path &archive, const string &expected) const
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
  if_.success("Hash is correct");
}

std::string Registry::pkg_url(const Target &target, const nlohmann::json &index)
{
  // TODO: optimise with get_key()
  if (!index.contains("packages"))
    throw ZCException(ZCE_MISSING_PROPERTY, "Index should contain hey 'packages'.");

  if (!index["packages"].contains(target.name))
    throw ZCException(ZCE_PKG_NOT_FOUND, "Package '" + target.name + "' not found.");

  if (!index["packages"][target.name]["versions"].contains(target.version.string()))
    throw ZCException(ZCE_NOT_FOUND, "Version " + target.version.string() + " does not exist.");

  return index["packages"][target.name]["versions"][target.version.string()].value("url", "");
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
