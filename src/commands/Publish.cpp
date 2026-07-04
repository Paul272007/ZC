#include "commands/Publish.h"

#include "commands/Command.h"
#include "Context.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Publish::Publish(const CommandContext &ctx) : Command(ctx) {}

void Publish::operator()()
{
  p().publish();
}

} // namespace zc
