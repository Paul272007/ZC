#pragma once

#include <expected>
#include <string>
#include <utility>

#include "core/Conf.hpp"
#include "core/project/Link.hpp"
#include "core/project/target/ProjectTargetDependency.hpp"
#include "core/project/target/ProjectTargetResolvedFlags.hpp"
#include "core/registry/target/RegistryTarget.hpp"

namespace zc { class Exception; } // namespace zc

namespace zc::core
{

class Project;

/// @brief A project build target
class ProjectTarget : public Conf
{
protected:
  Project *parent_;

  /// @brief Target root directory
  stdfs::path root_;
  /// @brief Target name
  std::string name_;

  std::map<std::string, std::string> public_macros_;
  std::map<std::string, std::string> private_macros_;

  std::map<std::string, Link>                   raw_internal_deps_;
  std::vector<std::pair<ProjectTarget *, Link>> internal_deps_;
  std::vector<ProjectTargetDependency>          external_deps_;

  explicit ProjectTarget(Project *parent, stdfs::path root);

  void load() override;

  void write() override;

  virtual void loadFromJson(const json &conf);

  virtual void writeToJson(json &conf) const;

  [[nodiscard]] stdfs::path installDir() const;

  [[nodiscard]] std::string formatIncludeFlag(const stdfs::path &dir) const;

  [[nodiscard]] std::expected<void, Exception> installHeaders() const;

  void fillRegistryTarget(RegistryTarget &target) const;

public:
  ProjectTarget(ProjectTarget &&)                 = delete;
  ProjectTarget(const ProjectTarget &)            = delete;
  ProjectTarget &operator=(ProjectTarget &&)      = delete;
  ProjectTarget &operator=(const ProjectTarget &) = delete;
  ~ProjectTarget() override                       = default;

  [[nodiscard]] static std::unique_ptr<ProjectTarget> fromRoot(stdfs::path root, Project *parent);

  [[nodiscard]] static std::unique_ptr<ProjectTarget>
  fromJson(stdfs::path root, const json &conf, Project *parent);

  // ——— Getters ———

  [[nodiscard]] const std::string &name() const noexcept { return name_; }

  [[nodiscard]] const stdfs::path &root() const noexcept { return root_; }

  [[nodiscard]] const auto &externalDeps() const noexcept { return external_deps_; }

  // ——— Interface ———

  [[nodiscard]] virtual std::expected<void, Exception> install() const = 0;

  [[nodiscard]] virtual std::vector<std::string> generateBuildStatements() const = 0;

  [[nodiscard]] virtual std::vector<stdfs::path> outputFiles() const = 0;

  [[nodiscard]] virtual std::unique_ptr<RegistryTarget> whenInstalled() const = 0;

  [[nodiscard]] virtual ProjectTargetResolvedFlags resolveFlags() const;

  void resolveInternalDeps(const std::vector<std::unique_ptr<ProjectTarget>> &project_targets);
};

} // namespace zc::core
