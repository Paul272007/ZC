#include "core/registry/Registry.hpp"

#include "core/project/Project.hpp"
#include "core/registry/package_index.hpp"
#include "core/util/directories.hpp"
#include "framework/Exception.hpp"
#include "framework/fs.hpp"
#include "framework/ui/UI.hpp"
#include "framework/util/string.hpp"

namespace zc::core
{

Registry::Registry() : Conf(zc_dir() / zc_registry_file)
{
  if (!fs::file_exists(file_))
    modified_ = true; // So that file is created in destructor
  else
    Registry::load();
}

RegistryPkg *Registry::addToIndex(RegistryPkg pkg)
{
  auto [it, inserted] = pkgs_.insert_or_assign(pkg.name, std::move(pkg));
  modified_           = true;
  return &it->second;
}

std::expected<RegistryPkg, Exception> Registry::removeFromIndex(const std::string_view name)
{
  const auto it_res = getPkgIt(name);
  if (!it_res)
    return std::unexpected(it_res.error());
  const auto &it = it_res.value();

  RegistryPkg extracted_pkg = std::move(it->second);
  pkgs_.erase(it);
  modified_ = true;
  return extracted_pkg;
}

std::expected<RegistryProject, Exception> Registry::removeFromIndex(std::string_view name, Version version)
{
  const auto it_res = getPkgIt(name);
  if (!it_res)
    return std::unexpected(it_res.error());
  auto &versions = it_res.value()->second.versions;

  for (u32 i = 0; i < versions.size(); i++) {
    auto &snapshot = versions[i];
    if (snapshot.version == version) {
      RegistryProject extracted_snapshot = std::move(snapshot);
      versions.erase(versions.begin() + i);
      modified_ = true;
      return extracted_snapshot;
    }
  }
  return std::unexpected(
    Exception(ZCError::VersionNotFound, "Version '{}' of package '{}' not found", version, name)
  );
}

std::expected<RegistryPkg *, Exception>
Registry::addVersion(const PkgMap::iterator &pkg, RegistryProject snapshot)
{
  pkg->second.versions.push_back(std::move(snapshot));
  modified_ = true;
  return &pkg->second;
}

std::expected<PkgMap::iterator, Exception> Registry::getPkgIt(const std::string_view name)
{
  auto it = pkgs_.find(name);
  if (it == pkgs_.end())
    return std::unexpected(Exception(ZCError::PkgNotFound, "Package '{}' not found", name));
  return it;
}

void Registry::clean()
{
  std::error_code   ec;
  const stdfs::path tmp = zc_dir() / zc_tmp_dir;
  if (fs::dir_exists(tmp))
    stdfs::remove_all(tmp, ec);
}

std::expected<RegistryPkg *, Exception>
Registry::finishInstallation(Project &p, const std::string &origin, const u32 jobs)
{
  const auto deps_install_res = p.installDependencies();
  if (!deps_install_res)
    return std::unexpected(deps_install_res.error());

  const auto build_res = p.installBuild(jobs);
  if (!build_res)
    return std::unexpected(build_res.error());

  const auto targets_install_res = p.installTargets();
  if (!targets_install_res)
    return std::unexpected(targets_install_res.error());

  std::vector<std::unique_ptr<RegistryTarget>> targets;
  targets.reserve(p.targets().size());
  for (const auto &target : p.targets())
    targets.emplace_back(target->whenInstalled());

  RegistryPkg pkg{
    .path            = "",
    .name            = p.name(),
    .origin          = origin,
    .default_version = p.version(),
    .versions        = {},
  };
  pkg.versions.push_back(
    RegistryProject{
      .description = p.description(),
      .author      = p.author(),
      .version     = p.version(),
      .targets     = std::move(targets),
    }
  );
  if (auto link_res = pkg.versions.front().createSymlinks(p.name()); !link_res)
    return std::unexpected(link_res.error());

  return addToIndex(std::move(pkg));
}

std::expected<RegistryPkg *, Exception> Registry::finishUpdate(
  Project &project, const PkgMap::iterator &pkg, const u32 jobs, const bool set_as_default
)
{
  const auto deps_install_res = project.installDependencies();
  if (!deps_install_res)
    return std::unexpected(deps_install_res.error());

  const auto build_res = project.installBuild(jobs);
  if (!build_res)
    return std::unexpected(build_res.error());

  const auto targets_install_res = project.installTargets();
  if (!targets_install_res)
    return std::unexpected(targets_install_res.error());

  std::vector<std::unique_ptr<RegistryTarget>> targets;
  targets.reserve(project.targets().size());
  for (const auto &target : project.targets())
    targets.push_back(target->whenInstalled());

  RegistryProject rp{
    .description = project.description(),
    .author      = project.author(),
    .version     = project.version(),
    .targets     = std::move(targets),
  };
  auto add_res = addVersion(pkg, std::move(rp));
  if (!add_res)
    return std::unexpected(add_res.error());

  if (set_as_default) {
    if (auto def_res = setDefaultVersion(project.name(), project.version()); !def_res)
      return std::unexpected(def_res.error());
  }

  return add_res;
}

std::map<std::string, Version, std::less<>> Registry::dependentsOf(const std::string_view name)
{
  std::map<std::string, Version, std::less<>> result;

  for (const auto &[pkg_name, pkg_decl] : pkgs_)
    for (const auto &snapshot : pkg_decl.versions)
      for (const auto &target : snapshot.targets)
        for (const auto &dep : target->external_deps)
          if (dep.package == name)
            result.insert_or_assign(pkg_name, snapshot.version);

  return result;
}

std::map<std::string, Version, std::less<>>
Registry::dependentsOf(const std::string_view name, const Version version)
{
  std::map<std::string, Version, std::less<>> result;

  for (const auto &[pkg_name, pkg_decl] : pkgs_)
    for (const auto &snapshot : pkg_decl.versions)
      for (const auto &target : snapshot.targets)
        for (const auto &dep : target->external_deps)
          if (dep.package == name && dep.version == version)
            result.insert_or_assign(pkg_name, snapshot.version);

  return result;
}

void Registry::load()
{
  if (const auto expected_root = fs::read_json(file_)) {
    if (const json &root = *expected_root; jsonContainsObject(root, "packages")) {
      pkgs_.clear();
      for (const auto &[key, value] : root["packages"].items()) {
        auto pkg = value.get<RegistryPkg>();
        pkg.name = key;
        pkgs_.insert_or_assign(key, std::move(pkg));
      }
    }
  }
}

void Registry::write()
{
  json root;
  json pkgs_json = json::object();
  for (const auto &[name, conf] : pkgs_)
    pkgs_json[name] = conf;
  root["packages"] = pkgs_json;
  if (!fs::write_json(file_, root)) {
    // TODO: return std::unexpected ?
  } else {
    modified_ = false;
  }
}

Registry::~Registry()
{
  try {
    clean();
    if (modified_)
      Registry::write();
  } catch (const std::exception &e) {
    ui.debug("{}", e.what());
  }
}

Registry &Registry::get()
{
  static Registry instance;
  return instance;
}

const RegistryPkg *Registry::getPkg(const std::string_view name) const
{
  const auto it = pkgs_.find(name);
  return it == pkgs_.end() ? nullptr : &it->second;
}

std::expected<void, Exception> Registry::setPath(const std::string_view name, const stdfs::path &path)
{
  const auto it_res = getPkgIt(name);
  if (!it_res)
    return std::unexpected(it_res.error());
  const auto it = it_res.value();

  it->second.path = path;
  modified_       = true;
  return {};
}

std::expected<void, Exception>
Registry::setDefaultVersion(const std::string_view name, const Version version)
{
  const auto it_res = getPkgIt(name);
  if (!it_res)
    return std::unexpected(it_res.error());
  const auto it = it_res.value();

  const RegistryProject *new_snapshot = nullptr;
  const RegistryProject *old_snapshot = nullptr;
  for (const auto &v : it->second.versions) {
    if (v.version == version)
      new_snapshot = &v;
    if (v.version == it->second.default_version)
      old_snapshot = &v;
  }

  if (!new_snapshot)
    return std::unexpected(
      Exception(ZCError::VersionNotFound, "Version '{}' of package '{}' not found", version, name)
    );

  if (old_snapshot) {
    if (auto rem_res = old_snapshot->removeSymlinks(name); !rem_res)
      return rem_res;
  }

  it->second.default_version = version;
  modified_                  = true;
  return new_snapshot->createSymlinks(name);
}

bool Registry::isInstalled(const std::string_view name) const
{
  return pkgs_.contains(name);
}

bool Registry::isInstalled(const std::string_view name, const Version version) const
{
  const auto it = pkgs_.find(name);
  if (it == pkgs_.end())
    return false;

  return std::ranges::any_of(it->second.versions, [version](const RegistryProject &rp) {
    return rp.version == version;
  });
}

std::expected<RegistryPkg *, Exception>
Registry::installStd(std::string name, std::vector<std::unique_ptr<RegistryTarget>> targets)
{
  RegistryPkg pkg{
    .path            = "",
    .name            = std::move(name),
    .origin          = "std",
    .default_version = {}, // empty version
    .versions        = {},
  };
  pkg.versions.push_back(
    RegistryProject{
      .description = "",
      .author      = "",
      .version     = {},
      .targets     = std::move(targets),
    }
  );
  return addToIndex(std::move(pkg));
}

std::expected<RegistryPkg *, Exception> Registry::installFromServer(
  const std::string_view name, const Version version, const u32 jobs, const bool force
)
{
  auto remote_pkg = index::resolve(name, version);
  if (!remote_pkg)
    return std::unexpected(remote_pkg.error());

  if (!force && isInstalled(remote_pkg->name, remote_pkg->version))
    return std::unexpected(Exception(
      ZCError::AlreadyInstalled, "Package {}@{} already installed", remote_pkg->name, remote_pkg->version
    ));

  auto project_dir = index::fetchAndExtract(*remote_pkg);
  if (!project_dir)
    return std::unexpected(project_dir.error());

  Project p(*project_dir);
  return finishInstallation(p, "main", jobs);
}

std::expected<RegistryPkg *, Exception> Registry::updateFromServer(
  const std::string_view name, const Version version, const u32 jobs, const bool force,
  const bool set_as_default
)
{
  const auto it_res = getPkgIt(name);
  if (!it_res)
    return std::unexpected(it_res.error());
  const auto &it = it_res.value();

  if (it->second.origin != "main")
    return std::unexpected(Exception(
      ZCError::OriginMismatch, "cannot update package of origin '{}' from server",
      it_res.value()->second.origin
    ));

  auto remote_pkg = index::resolve(name, version);
  if (!remote_pkg)
    return std::unexpected(remote_pkg.error());

  if (!force && isInstalled(remote_pkg->name, remote_pkg->version)) {
    if (version.isEmpty() || version.isLatest()) {
      return std::unexpected(
        Exception(ZCError::AlreadyInstalled, "package {} already up to date", remote_pkg->name)
      );
    }
    return std::unexpected(Exception(
      ZCError::AlreadyInstalled, "package {}@{} already installed", remote_pkg->name, remote_pkg->version
    ));
  }

  auto project_dir = index::fetchAndExtract(*remote_pkg);
  if (!project_dir)
    return std::unexpected(project_dir.error());

  Project p(*project_dir);
  return finishUpdate(p, it, jobs, set_as_default);
}

std::expected<RegistryPkg *, Exception>
Registry::installFromPath(Project &p, const u32 jobs, const bool save_path, const bool force)
{
  if (!force && isInstalled(p.name(), p.version()))
    return std::unexpected(
      Exception(ZCError::AlreadyInstalled, "package {}@{} already installed", p.name(), p.version())
    );

  const auto install_res = finishInstallation(p, "local", jobs);
  if (!install_res)
    return std::unexpected(install_res.error());

  if (save_path)
    install_res.value()->path = p.root();

  return install_res.value();
}

std::expected<RegistryPkg *, Exception> Registry::updateFromPath(
  Project &p, const u32 jobs, const bool save_path, const bool force, const bool set_as_default
)
{
  const auto it_res = getPkgIt(p.name());
  if (!it_res)
    return std::unexpected(it_res.error());
  const auto &it = it_res.value();

  if (!force && isInstalled(p.name(), p.version()))
    return std::unexpected(
      Exception(ZCError::AlreadyInstalled, "package {} already up to date", p.name(), p.version())
    );

  const auto update_res = finishUpdate(p, it, jobs, set_as_default);
  if (!update_res)
    return std::unexpected(update_res.error());
  auto *pkg = update_res.value();

  if (save_path)
    pkg->path = p.root();

  return pkg;
}

std::expected<RegistryProject, Exception> Registry::uninstallFromPath(const Project &p, const bool force)
{
  return uninstall(p.name(), p.version(), force);
}

std::expected<RegistryPkg, Exception> Registry::uninstall(const std::string_view name, const bool force)
{
  if (!force) {
    const auto dependents = dependentsOf(name);
    if (!dependents.empty()) {
      std::vector<std::string> dependents_vec;
      dependents_vec.reserve(dependents.size());
      for (const auto &[dep_name, dep_ver] : dependents)
        dependents_vec.push_back(std::format("{}@{}", dep_name, dep_ver));
      return std::unexpected(Exception(
        ZCError::BrokenDependency, "these packages depend on {}: {}", name, join(dependents_vec, ", ")
      ));
    }
  }
  auto pkg_res = removeFromIndex(name);
  if (!pkg_res)
    return std::unexpected(pkg_res.error());
  auto pkg = std::move(*pkg_res);

  for (const auto &v : pkg.versions) {
    if (v.version == pkg.default_version) {
      (void)v.removeSymlinks(name);
      break;
    }
  }

  const stdfs::path pkg_path = cache_dir() / name;
  if (fs::dir_exists(pkg_path))
    stdfs::remove_all(pkg_path);

  return pkg;
}

std::expected<RegistryProject, Exception>
Registry::uninstall(const std::string_view name, const Version version, const bool force)
{
  if (!force) {
    const auto dependents = dependentsOf(name, version);
    if (!dependents.empty()) {
      std::vector<std::string> dependents_vec;
      dependents_vec.reserve(dependents.size());
      for (const auto &[dep_name, dep_ver] : dependents)
        dependents_vec.push_back(std::format("{}@{}", dep_name, dep_ver));
      return std::unexpected(Exception(
        ZCError::BrokenDependency, "these packages depend on {}@{}: {}", name, version,
        join(dependents_vec, ", ")
      ));
    }
  }

  const auto it_res = getPkgIt(name);
  if (!it_res)
    return std::unexpected(it_res.error());
  const bool was_default = it_res.value()->second.default_version == version;

  auto snapshot_res = removeFromIndex(name, version);
  if (!snapshot_res)
    return std::unexpected(snapshot_res.error());
  auto snapshot = std::move(snapshot_res.value());

  if (was_default)
    (void)snapshot.removeSymlinks(name);

  const stdfs::path snapshot_path = cache_dir() / name / snapshot.version.string();
  if (fs::dir_exists(snapshot_path))
    stdfs::remove_all(snapshot_path);

  if (auto pkg_it = getPkgIt(name); pkg_it) {
    auto &versions = pkg_it.value()->second.versions;
    if (versions.empty()) {
      (void)removeFromIndex(name);
      const stdfs::path pkg_path = cache_dir() / name;
      if (fs::dir_exists(pkg_path))
        stdfs::remove_all(pkg_path);
    } else if (was_default) {
      const auto latest = std::ranges::max_element(versions, {}, &RegistryProject::version);
      (void)setDefaultVersion(name, latest->version);
    }
  }

  return snapshot;
}

} // namespace zc::core
