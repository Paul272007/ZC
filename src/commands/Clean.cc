
#include "commands/Clean.hh"

Clean::Clean(const bool force, const bool quiet) : Command(force, quiet), lc_(logger_, force)
{
}

int Clean::operator()()
{
  lc_.cleanProject();
  return 0;
}
