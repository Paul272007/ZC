#include "core/project/target/ProjectTarget.hpp"

#include "core/project/Project.hpp"
#include "core/project/target/ProjectBinTarget.hpp"

namespace zc::core
{

void ProjectTarget::load()
{
  auto res = fs::read_json(file_);
  if (!res)
    throw_or_terminate(res.error());
  loadFromJson(*res);
}

void ProjectTarget::loadFromJson(const json &conf)
{
  get_key_with_default(conf, "public_include_dirs", public_inc_dirs_, { name_ });
  get_key_with_default(conf, "private_include_dirs", private_inc_dirs_, { "include" });
  get_optional_key(conf, "public_macros", public_macros_);
  get_optional_key(conf, "private_macros", private_macros_);
  if (jsonContainsObject(conf, "dependencies")) {
    for (const auto &[k, v] : conf["dependencies"].items()) {
      auto dependency    = v.get<ProjectTargetDependency>();
      dependency.package = k;
      external_deps_.push_back(std::move(dependency));
    }
  }
  // TODO: fill internal_deps somewhere
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

  if (iequals(type, "BIN")) {
    target = std::make_unique<ProjectBinTarget>(parent, std::move(root));
    // TODO: add other target types
  } else {
    throw_or_terminate(Exception(ZCError::InvalidValue, "Unknown target type '{}'", type));
  }

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
        flags.macros.push_back("-D" + k + "=" + v);
  };

  const auto add_includes = [&](const std::vector<std::string> &inc_dirs) {
    for (const auto &inc : inc_dirs) {
      stdfs::path p = stdfs::path(inc).is_absolute() ? stdfs::path(inc) : root_ / inc;
      if (parent_) {
        std::error_code ec;
        auto            rel = stdfs::relative(p, parent_->root(), ec);
        if (!ec && !rel.empty()) {
          flags.includes.push_back("-I" + rel.string());
          continue;
        }
      }
      flags.includes.push_back("-I" + p.string());
    }
  };

  add_macros(public_macros_);
  add_macros(private_macros_);
  add_includes(public_inc_dirs_);
  add_includes(private_inc_dirs_);

  return flags;
}

} // namespace zc::core
