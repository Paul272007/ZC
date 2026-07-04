#include "Use.h"

#include "commands/Command.h"
#include "Context.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Use::Use(const CommandContext &ctx, const vector<string> &targets, const bool global)
  : Command(ctx, !global), global_(global)
{
  auto parsed_targets = parse_targets(targets);
  for (const auto &t : parsed_targets)
    targets_.push_back(LocalTarget::get_target(t));
}

void Use::operator()()
{
  if (global_)
    for (CAA[name, origin, new_version] : targets_)
      rg().set_default_version(name, new_version);
  else
    for (CAA[name, origin, new_version] : targets_)
      p().change_dependency_version(name, new_version);
}

} // namespace zc
