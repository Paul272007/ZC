#pragma once

#include "ProjectTarget.hpp"

namespace zc::core
{

class ProjectHeaderTarget final : public ProjectTarget
{
protected:
  void load() override { ProjectTarget::load(); }

  void write() override { ProjectTarget::write(); }

public:
  ProjectHeaderTarget(ProjectHeaderTarget &&)                 = delete;
  ProjectHeaderTarget(const ProjectHeaderTarget &)            = delete;
  ProjectHeaderTarget &operator=(ProjectHeaderTarget &&)      = delete;
  ProjectHeaderTarget &operator=(const ProjectHeaderTarget &) = delete;
  ~ProjectHeaderTarget() override                             = default;

  explicit ProjectHeaderTarget(Project *parent, stdfs::path root = stdfs::current_path())
    : ProjectTarget(parent, std::move(root))
  {
  }

  [[nodiscard]] constexpr std::string_view typeName() const noexcept override { return "HEADER"; }

  [[nodiscard]] std::expected<void, Exception>  install() const override;
  [[nodiscard]] std::vector<std::string>        generateBuildStatements() const override;
  [[nodiscard]] std::vector<stdfs::path>        outputFiles() const override;
  [[nodiscard]] std::unique_ptr<RegistryTarget> whenInstalled() const override;
};

} // namespace zc::core

// zc
