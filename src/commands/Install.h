#pragma once

#include "commands/BuildCommand.h"
#include "commands/InstallCommand.h"
#include "Context.h"

namespace zc
{

class Install : public BuildCommand, public InstallCommand
{
public:
  Install(
    CommandContext &c_ctx, const BuildContext &b_ctx, InstallContext &i_ctx, bool is_std, bool save_path
  );

  void operator()() override;

private:
  const bool std_;
  const bool save_path_;

  void install_from_path();
  void install_dependencies();
  void install_targets();
};

} // namespace zc
