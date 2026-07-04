#include "commands/Clean.h"

#include "commands/Command.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Clean::Clean(const CommandContext &ctx) : Command(ctx) {}

void Clean::operator()()
{
  p().clean();
}

} // namespace zc
