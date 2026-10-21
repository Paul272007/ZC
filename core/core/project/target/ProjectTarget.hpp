#pragma once

#include <expected>
#include <string>
#include <utility>

#include "core/Conf.hpp"
#include "core/project/Language.hpp"
#include "core/project/target/ProjectTargetDependency.hpp"
#include "core/project/target/ProjectTargetResolvedFlags.hpp"
#include "core/registry/target/RegistryTarget.hpp"
#include "core/util/directories.hpp"
#include "framework/Exception.hpp"
#include "framework/util/string.hpp"

namespace zc::core
{

class Project;

using SourceMap = std::map<Language, std::vector<stdfs::path>>;

/// @brief A project build target
class ProjectTarget : public Conf
{
protected:
  Project *parent_;

  /// @brief Target root directory
  stdfs::path root_;
  /// @brief Target name
  std::string name_;

  std::vector<std::string> public_inc_dirs_;
  std::vector<std::string> private_inc_dirs_;

  std::map<std::string, std::string> public_macros_;
  std::map<std::string, std::string> private_macros_;

  // TODO: change to map to allow static linking
  std::vector<ProjectTarget *>         internal_deps_;
  std::vector<ProjectTargetDependency> external_deps_;

  ProjectTarget(Project *parent, stdfs::path root)
    : Conf(root / zc_target_file), parent_(parent), root_(std::move(root)), name_(root_.filename().string())
  {
    // TODO: check folders structure and target name
  }

  void load() override;

  virtual void loadFromJson(const json &conf);

public:
  ProjectTarget(ProjectTarget &&)                 = delete;
  ProjectTarget(const ProjectTarget &)            = delete;
  ProjectTarget &operator=(ProjectTarget &&)      = delete;
  ProjectTarget &operator=(const ProjectTarget &) = delete;
  ~ProjectTarget() override                       = default;

  [[nodiscard]] static std::unique_ptr<ProjectTarget> fromRoot(stdfs::path root, Project *parent);

  [[nodiscard]] static std::unique_ptr<ProjectTarget>
  fromJson(stdfs::path root, const json &conf, Project *parent);

  [[nodiscard]] const std::string &name() const noexcept { return name_; }

  [[nodiscard]] const stdfs::path &root() const noexcept { return root_; }

  [[nodiscard]] virtual ProjectTargetResolvedFlags resolveFlags() const;

  [[nodiscard]] virtual std::vector<std::string> generateBuildStatements() const = 0;

  [[nodiscard]] virtual std::vector<stdfs::path> outputFiles() const = 0;

  [[nodiscard]] virtual std::unique_ptr<RegistryTarget> toRegistryTarget() const = 0;

  virtual std::expected<void, Exception> build()   = 0;
  virtual std::expected<void, Exception> install() = 0;
};

} // namespace zc::core
