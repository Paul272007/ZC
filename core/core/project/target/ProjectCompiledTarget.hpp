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

  void writeToJson(json &conf) const override;

  [[nodiscard]] virtual bool isOutputValid() const;

  [[nodiscard]] SourceMap findSources() const;

  [[nodiscard]] std::vector<std::string> compileObjectStatements(
    const SourceMap &sources, const ProjectTargetResolvedFlags &flags, std::vector<std::string> &statements,
    bool pic = false
  ) const;

  [[nodiscard]] std::string resolveLinker(const SourceMap &sources) const;

  [[nodiscard]] std::string formatLinkStatement(
    const stdfs::path &out_rel, const std::vector<std::string> &objs, const SourceMap &sources,
    const ProjectTargetResolvedFlags &flags, bool shared = false
  ) const;

public:
  ProjectCompiledTarget(ProjectCompiledTarget &&)                 = delete;
  ProjectCompiledTarget(const ProjectCompiledTarget &)            = delete;
  ProjectCompiledTarget &operator=(ProjectCompiledTarget &&)      = delete;
  ProjectCompiledTarget &operator=(const ProjectCompiledTarget &) = delete;
  ~ProjectCompiledTarget() override                               = default;

  [[nodiscard]] ProjectTargetResolvedFlags resolveFlags() const override;
};

} // namespace zc::core

// zc
