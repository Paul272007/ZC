#pragma once

#include "commands/BuildCommand.h"
#include "commands/InstallCommand.h"
#include "Context.h"

namespace zc
{

class Update : public BuildCommand, public InstallCommand
{
public:
  Update(CommandContext &c_ctx, BuildContext &b_ctx, InstallContext &i_ctx, bool dont_use, bool save_path);

  void operator()() override;

private:
  const bool use_;
  const bool save_path_;

  void update_from_path();
  void update_dependencies();
  void update_targets();
};

} // namespace zc
