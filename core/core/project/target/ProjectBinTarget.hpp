#pragma once

#include <string_view>

#include "core/project/Project.hpp"
#include "core/project/target/ProjectCompiledTarget.hpp"
#include "framework/shell.hpp"

namespace zc::core
{

class ProjectBinTarget final : public ProjectCompiledTarget
{
  [[nodiscard]] std::string binFmt() const;

protected:
  void load() override { ProjectTarget::load(); }

  void write() override { ProjectTarget::write(); }

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

  [[nodiscard]] constexpr std::string_view typeName() const noexcept override { return "BIN"; }

  [[nodiscard]] std::expected<void, Exception> install() const override;

  [[nodiscard]] std::vector<std::string> generateBuildStatements() const override;

  [[nodiscard]] std::vector<stdfs::path> outputFiles() const override;

  [[nodiscard]] std::unique_ptr<RegistryTarget> whenInstalled() const override;

  [[nodiscard]] int
  execute(const std::vector<std::string_view> &args, sh::Output output = sh::Output::ShowAll) const;
};

} // namespace zc::core
