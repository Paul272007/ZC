#pragma once

#include <filesystem>
#include <memory>

#include "commands/Command.h"
#include "project/Project.h"

namespace zc
{

class ProjectCommand : public Command
{
public:
  ProjectCommand(const ProjectCommand &)            = delete;
  ProjectCommand(ProjectCommand &&)                 = delete;
  ProjectCommand &operator=(const ProjectCommand &) = delete;
  ProjectCommand &operator=(ProjectCommand &&)      = delete;
  ~ProjectCommand() override                        = default;

protected:
  std::unique_ptr<Project> project_;

  explicit ProjectCommand(
    const bool force, const std::filesystem::path &p_root = std::filesystem::current_path(),
    const bool require_project = true
  )
    : Command(force)
  {
    if (require_project)
      project_ = std::make_unique<Project>(get_project_root(p_root));
  }

  [[nodiscard]] Project &p() { return *project_; }

  [[nodiscard]] const Project &p() const { return *project_; }

  [[nodiscard]] bool has_project() const { return project_ != nullptr; }
};

} // namespace zc
