#pragma once

#include "core/project/target/ProjectCompiledTarget.hpp"

namespace zc::core
{

class ProjectLibTarget final : public ProjectCompiledTarget
{
  [[nodiscard]] std::expected<void, Exception> installLibs() const;

protected:
  void load() override { ProjectTarget::load(); }

  void write() override { ProjectTarget::write(); }

public:
  ProjectLibTarget(ProjectLibTarget &&)                 = delete;
  ProjectLibTarget(const ProjectLibTarget &)            = delete;
  ProjectLibTarget &operator=(ProjectLibTarget &&)      = delete;
  ProjectLibTarget &operator=(const ProjectLibTarget &) = delete;
  ~ProjectLibTarget() override                          = default;

  explicit ProjectLibTarget(Project *parent, stdfs::path root = stdfs::current_path())
    : ProjectCompiledTarget(parent, std::move(root))
  {
  }

  [[nodiscard]] std::string sharedFmt() const;
  [[nodiscard]] std::string staticFmt() const;

  [[nodiscard]] std::expected<void, Exception> install() const override;

  [[nodiscard]] std::vector<std::string> generateBuildStatements() const override;

  [[nodiscard]] std::vector<stdfs::path> outputFiles() const override;

  [[nodiscard]] std::unique_ptr<RegistryTarget> whenInstalled() const override;
};

} // namespace zc::core
