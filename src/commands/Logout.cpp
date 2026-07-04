#include "commands/Logout.h"

namespace zc
{

Logout::Logout(const CommandContext &ctx) : Command(ctx, false) {}

void Logout::operator()()
{
  gc_.logout();
}

} // namespace zc
