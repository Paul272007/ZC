#include "commands/Logout.h"

namespace zc
{

Logout::Logout(const bool force) : Command(force)
{
}

void Logout::operator()()
{
  gc_.logout();
}

} // namespace zc
