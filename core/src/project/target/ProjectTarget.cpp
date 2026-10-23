#include "core/project/target/ProjectTarget.hpp"

#include "core/project/Project.hpp"
#include "core/project/target/ProjectBinTarget.hpp"
#include "core/project/target/ProjectHeaderTarget.hpp"
#include "core/project/target/ProjectLibTarget.hpp"
#include "core/registry/Registry.hpp"
#include "core/registry/target/RegistryLibTarget.hpp"

namespace zc::core
{

ProjectTarget::ProjectTarget(Project *parent, stdfs::path root)
  : Conf(root / zc_target_file),
    parent_(parent),
    root_(std::move(root)),
    name_(parent_ && root_ == parent_->root() ? parent_->name() : root_.filename().string())
{
  // TODO: check folders structure and target name
}

void ProjectTarget::load()
{
  auto res = fs::read_json(file_);
  if (!res)
    throw_or_terminate(res.error());
  loadFromJson(*res);
}

void ProjectTarget::write()
{
  json root;
  writeToJson(root);
  auto res = fs::write_json(file_, root);
  if (!res)
    throw_or_terminate(res.error());
}

void ProjectTarget::loadFromJson(const json &conf)
{
  get_optional_key(conf, "public_macros", public_macros_);
  get_optional_key(conf, "private_macros", private_macros_);
  get_optional_key(conf, "internal_deps", raw_internal_deps_);
  if (jsonContainsObject(conf, "dependencies")) {
    for (const auto &[k, v] : conf["dependencies"].items()) {
      auto dependency    = v.get<ProjectTargetDependency>();
      dependency.package = k;
      external_deps_.push_back(std::move(dependency));
    }
  }
}

void ProjectTarget::writeToJson(json &conf) const
{
  conf["public_macros"]  = public_macros_;
  conf["private_macros"] = private_macros_;
  conf["internal_deps"]  = raw_internal_deps_;
  conf["dependencies"]   = json::object();
  for (const auto &dep : external_deps_)
    conf["dependencies"][dep.package] = dep;
}

stdfs::path ProjectTarget::installDir() const
{
  return cache_dir() / parent_->name() / parent_->version().string();
}

std::string ProjectTarget::formatIncludeFlag(const stdfs::path &dir) const
{
  if (parent_) {
    std::error_code ec;
    auto            rel = stdfs::relative(dir, parent_->root(), ec);
    if (!ec && !rel.empty())
      return "-I" + rel.string();
  }
  return "-I" + dir.string();
}

std::expected<void, Exception> ProjectTarget::installHeaders() const
{
  const stdfs::path src  = root_ / zc_include_dir / name_;
  const stdfs::path dest = installDir() / zc_include_dir / parent_->name() / name_;

  if (!fs::dir_exists(src))
    return std::unexpected(
      Exception(FSError::DirNotFound, "expected public header directory not found: {}", src)
    );

  std::error_code ec;
  stdfs::create_directories(installDir() / zc_include_dir / parent_->name(), ec);
  stdfs::copy(src, dest, stdfs::copy_options::overwrite_existing | stdfs::copy_options::recursive, ec);
  if (ec)
    return std::unexpected(
      Exception(FSError::CopyError, "failed to install headers to {}: {}", dest, ec.message())
    );
  return {};
}

void ProjectTarget::fillRegistryTarget(RegistryTarget &target) const
{
  std::vector<std::string> internal_deps;
  internal_deps.reserve(internal_deps_.size());
  for (const auto &[dep, link_mode] : internal_deps_)
    if (link_mode == Link::Dynamic)
      internal_deps.push_back(dep->name());

  std::vector<RegistryTargetDependency> external_deps;
  external_deps.reserve(external_deps_.size());
  for (const auto &dep : external_deps_)
    external_deps.push_back(dep.toRegistryTargetDependency());

  target.name          = name_;
  target.macros        = public_macros_;
  target.internal_deps = std::move(internal_deps);
  target.external_deps = std::move(external_deps);
}

std::unique_ptr<ProjectTarget> ProjectTarget::fromRoot(stdfs::path root, Project *parent)
{
  auto res = fs::read_json(root / zc_target_file);
  if (!res)
    throw_or_terminate(res.error());
  return fromJson(std::move(root), *res, parent);
}

std::unique_ptr<ProjectTarget> ProjectTarget::fromJson(stdfs::path root, const json &conf, Project *parent)
{
  std::string type;
  GET_KEY(conf, type);
  std::unique_ptr<ProjectTarget> target;

  if (iequals(type, "BIN"))
    target = std::make_unique<ProjectBinTarget>(parent, std::move(root));
  else if (iequals(type, "LIB"))
    target = std::make_unique<ProjectLibTarget>(parent, std::move(root));
  else if (iequals(type, "HEADER"))
    target = std::make_unique<ProjectHeaderTarget>(parent, std::move(root));
  else
    throw_or_terminate(Exception(ZCError::InvalidValue, "Unknown target type '{}'", type));

  target->loadFromJson(conf);
  return target;
}

ProjectTargetResolvedFlags ProjectTarget::resolveFlags() const
{
  ProjectTargetResolvedFlags flags;

  const auto add_macros = [&](const std::map<std::string, std::string> &macros) {
    for (const auto &[k, v] : macros)
      if (v.empty())
        flags.macros.push_back("-D" + k);
      else
        flags.macros.push_back(std::format("-D{}={}", k, v));
  };

  add_macros(public_macros_);
  add_macros(private_macros_);

  if (fs::dir_exists(root_ / zc_include_dir))
    flags.includes.push_back(formatIncludeFlag(root_ / zc_include_dir));

  for (const auto &[dep, link_mode] : internal_deps_) {
    add_macros(dep->public_macros_);
    if (fs::dir_exists(dep->root_ / zc_include_dir))
      flags.includes.push_back(formatIncludeFlag(dep->root_ / zc_include_dir));

    if (const auto *lib_dep = dynamic_cast<const ProjectLibTarget *>(dep)) {
      if (link_mode == Link::Static) {
        const stdfs::path static_rel = stdfs::path(zc_build_dir) / lib_dep->name() / lib_dep->staticFmt();
        flags.static_libs.push_back(static_rel.string());
        flags.implicit_deps.push_back(static_rel.string());
      } else {
        const stdfs::path shared_rel = stdfs::path(zc_build_dir) / lib_dep->name() / lib_dep->sharedFmt();
        flags.ldflags.push_back(shared_rel.string());
        flags.implicit_deps.push_back(shared_rel.string());
      }
    }
  }

  for (const auto &dep : external_deps_) {
    const auto *pkg = Registry::get().getPkg(dep.package);
    if (!pkg)
      throw_or_terminate(Exception(ZCError::PkgNotFound, "Package '{}' not found", dep.package));

    const Version          ver      = dep.version == Version{} ? pkg->default_version : dep.version;
    const RegistryProject *snapshot = nullptr;
    for (const auto &v : pkg->versions) {
      if (v.version == ver) {
        snapshot = &v;
        break;
      }
    }
    if (!snapshot)
      throw_or_terminate(
        Exception(ZCError::VersionNotFound, "Version '{}' of package '{}' not found", ver, dep.package)
      );

    const stdfs::path pkg_dir = cache_dir() / dep.package / ver.string();
    if (fs::dir_exists(pkg_dir / zc_include_dir))
      flags.includes.push_back("-I" + (pkg_dir / zc_include_dir).string());

    for (const auto &[target_name, link_mode] : dep.targets) {
      const RegistryTarget *reg_target = nullptr;
      for (const auto &t : snapshot->targets) {
        if (t->name == target_name) {
          reg_target = t.get();
          break;
        }
      }
      if (!reg_target)
        throw_or_terminate(Exception(
          ZCError::TargetNotFound, "Target '{}' not found in package '{}@{}'", target_name, dep.package, ver
        ));

      add_macros(reg_target->macros);

      if (const auto *lib_target = dynamic_cast<const RegistryLibTarget *>(reg_target)) {
        if (link_mode == Link::Static)
          flags.static_libs.push_back((pkg_dir / zc_lib_dir / lib_target->static_output).string());
        else
          flags.ldflags.push_back((pkg_dir / zc_lib_dir / lib_target->shared_output).string());
      }
    }
  }

  return flags;
}

void ProjectTarget::resolveInternalDeps(const std::vector<std::unique_ptr<ProjectTarget>> &project_targets)
{
  internal_deps_.clear();
  for (const auto &[dep_name, link_mode] : raw_internal_deps_) {
    ProjectTarget *found = nullptr;
    for (const auto &t : project_targets) {
      if (t->name() == dep_name && t.get() != this) {
        found = t.get();
        break;
      }
    }
    if (!found)
      throw_or_terminate(
        Exception(ZCError::TargetNotFound, "internal dependency '{}' not found in project", dep_name)
      );
    internal_deps_.emplace_back(found, link_mode);
  }
}

} // namespace zc::core
