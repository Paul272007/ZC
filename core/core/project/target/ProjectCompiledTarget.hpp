#pragma once

#include <utility>

#include "core/project/Language.hpp"
#include "core/project/target/ProjectTarget.hpp"

namespace zc::core
{

class ProjectCompiledTarget : public ProjectTarget
{
protected:
  std::string output_;

  std::vector<stdfs::path> src_dirs_;

  explicit ProjectCompiledTarget(Project *parent, stdfs::path root) : ProjectTarget(parent, std::move(root))
  {
  }

  void loadFromJson(const json &conf) override;

  [[nodiscard]] virtual bool isOutputValid() const = 0;

  [[nodiscard]] static Language linker(const SourceMap &sources);

  [[nodiscard]] SourceMap findSources() const;

public:
  ProjectCompiledTarget(ProjectCompiledTarget &&)                 = delete;
  ProjectCompiledTarget(const ProjectCompiledTarget &)            = delete;
  ProjectCompiledTarget &operator=(ProjectCompiledTarget &&)      = delete;
  ProjectCompiledTarget &operator=(const ProjectCompiledTarget &) = delete;
  ~ProjectCompiledTarget() override                               = default;

  [[nodiscard]] std::vector<stdfs::path> outputFiles() const override = 0;
};

} // namespace zc::core

// zc
