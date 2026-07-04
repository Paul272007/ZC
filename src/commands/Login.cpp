#include "commands/Login.h"

namespace zc
{

Login::Login(const CommandContext &ctx) : Command(ctx, false) {}

void Login::operator()()
{
  gc_.login(force_);
}

} // namespace zc
