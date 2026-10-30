#pragma once

#include <cstdio>
#include <filesystem>
#include <memory>
#include <tuple>

#include "core/project/Project.hpp"
#include "framework/cli/Args.hpp"
#include "framework/cli/CommandBase.hpp"
#include "framework/Exception.hpp"
#include "framework/ui/UI.hpp"
#include "framework/util/helpers.hpp"

namespace zc::app
{

/// @brief Base class of ZC's self-describing commands.
///
/// Owns the common flags (--quiet, --verbose, --force, --input, --output, --error, --project-path)
/// and handles streams, UI initialization and project loading before running `run()`.
template<cli::FixedString Name, cli::FixedString Desc, bool RequireProject = false>
class AppCommand : public cli::CommandBase<Name, Desc>
{
  std::unique_ptr<core::Project> project_;

protected:
  ZC_FLAG(quiet, 'q', "Enable quiet mode");
  ZC_FLAG(verbose, 'v', "Enable verbosity");
  ZC_FLAG(force, 'f', "Force execution and skip usual warnings");

  ZC_OPTION(input, stdfs::path, 'i', "Input file", {});
  ZC_OPTION(output, stdfs::path, 'o', "Output file", {});
  ZC_OPTION(error, stdfs::path, 'e', "Error file", {});
  ZC_OPTION(project_path, stdfs::path, 'p', "Path to project", stdfs::current_path());

  AppCommand() = default;

  [[nodiscard]] core::Project &p() { return *project_; }

  [[nodiscard]] const core::Project &p() const { return *project_; }

  [[nodiscard]] bool hasProject() const noexcept { return project_ != nullptr; }

  [[nodiscard]] virtual bool requiresProject() const { return RequireProject; }

  virtual void run() = 0;

public:
  AppCommand(AppCommand &&)                 = delete;
  AppCommand(const AppCommand &)            = delete;
  AppCommand &operator=(AppCommand &&)      = delete;
  AppCommand &operator=(const AppCommand &) = delete;
  virtual ~AppCommand()                     = default;

  template<typename Self>
  [[nodiscard]] auto allArgs(this Self &self)
  {
    auto &base   = static_cast<AppCommand &>(self);
    auto  common = std::tie(
      base.quiet_, base.verbose_, base.force_, base.input_, base.output_, base.error_, base.project_path_
    );
    if constexpr (requires { self.options(); })
      return std::tuple_cat(self.options(), common);
    else
      return common;
  }

  void execute()
  {
    FILE *in = input_->empty() ? stdin : std::fopen(input_->c_str(), "r");
    if (!in)
      throw_or_terminate(Exception(FSError::ReadingError, "failed to open stream {}", *input_));

    FILE *out = output_->empty() ? stdout : std::fopen(output_->c_str(), "w");
    if (!out)
      throw_or_terminate(Exception(FSError::WritingError, "failed to open stream {}", *output_));

    FILE *err = error_->empty() ? stderr : std::fopen(error_->c_str(), "w");
    if (!err)
      throw_or_terminate(Exception(FSError::WritingError, "failed to open stream {}", *error_));

    const auto mode = parse_mode<UIMode>(
      {
        { UIMode::Quiet, *quiet_ },
        { UIMode::Verbose, *verbose_ },
      },
      UIMode::Normal, "incompatible flags --quiet and --verbose"
    );
    UI::init(in, out, err, mode);

    if (requiresProject()) {
      auto expected_root = core::project_root_dir(*project_path_);
      if (!expected_root)
        throw_or_terminate(expected_root.error());
      project_ = std::make_unique<core::Project>(*expected_root);
    }

    run();
  }
};

} // namespace zc::app
