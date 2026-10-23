#pragma once

#include <memory>

#include "context/CommandCtx.hpp"
#include "core/project/Project.hpp"
#include "framework/ui/UI.hpp"

namespace zc::app
{

struct CommandCtx;

class Command
{
  std::unique_ptr<core::Project> project_;

protected:
  const bool force_;

  explicit Command(const CommandCtx &ctx, const bool require_project) : force_(ctx.force)
  {
    FILE *in = ctx.in_path.empty() ? stdin : std::fopen(ctx.in_path.c_str(), "r");
    if (!in)
      throw_or_terminate(Exception(FSError::ReadingError, "failed to open stream {}", ctx.in_path));

    FILE *out = ctx.out_path.empty() ? stdout : std::fopen(ctx.out_path.c_str(), "w");
    if (!out)
      throw_or_terminate(Exception(FSError::WritingError, "failed to open stream {}", ctx.out_path));

    FILE *err = ctx.err_path.empty() ? stderr : std::fopen(ctx.err_path.c_str(), "w");
    if (!err)
      throw_or_terminate(Exception(FSError::WritingError, "failed to open stream {}", ctx.err_path));

    const auto mode = parse_mode<UIMode>(
      {
        { UIMode::Quiet, ctx.quiet },
        { UIMode::Verbose, ctx.verbose },
      },
      UIMode::Normal, "incompatible flags --quiet and --verbose"
    );
    UI::init(in, out, err, mode);

    if (require_project) {
      auto expected_root = core::project_root_dir(ctx.project_path);
      if (!expected_root)
        throw_or_terminate(expected_root.error());
      project_ = std::make_unique<core::Project>(*expected_root);
    }
  }

  [[nodiscard]] core::Project &p() { return *project_; }

  [[nodiscard]] const core::Project &p() const { return *project_; }

  [[nodiscard]] bool hasProject() const noexcept { return project_ != nullptr; }

public:
  Command(const Command &)            = delete;
  Command &operator=(const Command &) = delete;
  Command(Command &&)                 = delete;
  Command &operator=(Command &&)      = delete;
  virtual ~Command()                  = default;

  virtual void execute() = 0;
};

} // namespace zc::app
