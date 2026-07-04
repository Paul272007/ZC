#pragma once

#include "config/GConf.h"
#include "Context.h"
#include "project/Project.h"
#include "ui/Interface.h"

namespace zc
{

class Command
{
public:
  Command(const Command &)            = delete;
  Command &operator=(const Command &) = delete;
  Command(Command &&)                 = delete;
  Command &operator=(Command &&)      = delete;

  virtual ~Command() = default;

  virtual void operator()() = 0;

protected:
  GConf     &gc_ = GConf::get();
  Interface &if_ = Interface::get(); // Could be const since it practically only has const methods
  const bool force_;
  std::unique_ptr<Project> project_;

  explicit Command(const CommandContext &ctx, bool require_project = true) : force_(ctx.force)
  {
    if (require_project)
      project_ = std::make_unique<Project>(get_project_root(ctx.p_root));
  }

  [[nodiscard]] Project &p() { return *project_; }

  [[nodiscard]] const Project &p() const { return *project_; }

  [[nodiscard]] bool has_project() const { return project_ != nullptr; }
};

} // namespace zc
