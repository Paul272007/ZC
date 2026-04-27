#include "commands/Logout.hh"
#include "commands/Command.hh"

Logout::Logout() : Command(false, false), g_(logger_, false)
{
}

int Logout::operator()()
{
  g_.logout();
  return 0;
}
