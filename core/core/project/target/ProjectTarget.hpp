#pragma once

#include <expected>
#include <string>

#include "core/project/target/ProjectTargetDependency.hpp"
#include "core/registry/target/RegistryTarget.hpp"
#include "framework/Exception.hpp"

namespace zc::core
{

/// @brief A project build target
class ProjectTarget
{
  /// @brief Target root directory
  stdfs::path root_;
  /// @brief Target name
  std::string name_;

  std::vector<stdfs::path> src_dirs_;
  std::vector<stdfs::path> public_inc_dirs_;
  std::vector<stdfs::path> private_inc_dirs_;

  std::map<std::string, std::string> public_macros_;
  std::map<std::string, std::string> private_macros_;

  std::vector<ProjectTarget *>         internal_deps_;
  std::vector<ProjectTargetDependency> external_deps_;

public:
  ProjectTarget(ProjectTarget &&)                 = delete;
  ProjectTarget(const ProjectTarget &)            = delete;
  ProjectTarget &operator=(ProjectTarget &&)      = delete;
  ProjectTarget &operator=(const ProjectTarget &) = delete;
  virtual ~ProjectTarget()                        = default;

  [[nodiscard]] const std::string &name() const noexcept { return name_; }

  [[nodiscard]] virtual std::unique_ptr<RegistryTarget> toRegistryTarget() const = 0;
  //{
  // std::vector<std::string> internal_deps;
  // internal_deps.reserve(internal_deps_.size());
  // for (const auto &dep: internal_deps_)
  // internal_deps.push_back(dep->name());
  //
  // std::vector<RegistryTargetDependency> external_deps;
  // external_deps.reserve(external_deps_.size());
  // for (const auto &dep: external_deps_)
  // external_deps.push_back(dep.toRegistryTargetDependency());
  //
  // return RegistryTarget{
  // .name = name_,
  // .inc_dirs = public_inc_dirs_,
  // .macros        = public_macros_,
  // .internal_deps = std::move(internal_deps),
  // .external_deps = std::move(external_deps),
  // };
  //};

  virtual std::expected<void, Exception> build()   = 0;
  virtual std::expected<void, Exception> install() = 0;
};

} // namespace zc::core
