#include "commands/Logout.h"

namespace zc
{

Logout::Logout(const bool force) : Command(force)
{
}

int Logout::operator()()
{
  gc_.logout();
  return 0;
}

} // namespace zc
