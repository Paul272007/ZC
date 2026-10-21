#pragma once

#include "core/project/Project.hpp"
#include "core/project/target/ProjectCompiledTarget.hpp"
#include "core/registry/target/RegistryBinTarget.hpp"
#include "framework/shell.hpp"

namespace zc::core
{

class ProjectBinTarget final : public ProjectCompiledTarget
{
  [[nodiscard]] stdfs::path installDir() const;

  [[nodiscard]] std::string binFmt() const;

protected:
  void load() override;

  void write() override;

  [[nodiscard]] bool isOutputValid() const override;

public:
  ProjectBinTarget(ProjectBinTarget &&)                 = delete;
  ProjectBinTarget(const ProjectBinTarget &)            = delete;
  ProjectBinTarget &operator=(ProjectBinTarget &&)      = delete;
  ProjectBinTarget &operator=(const ProjectBinTarget &) = delete;
  ~ProjectBinTarget() override                          = default;

  explicit ProjectBinTarget(Project *parent, stdfs::path root = stdfs::current_path())
    : ProjectCompiledTarget(parent, std::move(root))
  {
  }

  [[nodiscard]] std::unique_ptr<RegistryTarget> toRegistryTarget() const override;

  std::expected<void, Exception> build() override;

  std::expected<void, Exception> install() override;

  [[nodiscard]] std::vector<stdfs::path> outputFiles() const override;

  [[nodiscard]] std::vector<std::string> generateBuildStatements() const override;

  [[nodiscard]] int
  execute(const std::vector<std::string> &args, sh::Output output = sh::Output::ShowAll) const;
};

} // namespace zc::core
