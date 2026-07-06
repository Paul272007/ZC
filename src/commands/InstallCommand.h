#pragma once

#include <filesystem>
#include <optional>

#include "commands/Command.h"
#include "Context.h"
#include "helpers.h"
#include "pkgs/Registry.h"

namespace zc
{

class InstallCommand : public Command
{
public:
  InstallCommand(const InstallCommand &)            = delete;
  InstallCommand(InstallCommand &&)                 = delete;
  InstallCommand &operator=(const InstallCommand &) = delete;
  InstallCommand &operator=(InstallCommand &&)      = delete;
  ~InstallCommand() override                        = default;

protected:
  Registry  &reg_ = Registry::get();
  const bool sync_;

  const std::filesystem::path path_;
  const std::vector<Target>   targets_;

  explicit InstallCommand(CommandContext &c_ctx, InstallContext &i_ctx, std::optional<bool> require_project = std::nullopt)
    : Command(c_ctx, require_project.value_or((i_ctx.targets.empty() && i_ctx.path.empty()) || i_ctx.sync)),
      sync_(i_ctx.sync),
      path_(std::move(i_ctx.path)),
      targets_(parse_targets(i_ctx.targets))
  {
  }
};

} // namespace zc
