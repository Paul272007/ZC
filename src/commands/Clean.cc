#include "commands/Clean.hh"
#include "helpers.hh"

Clean::Clean(bool force, bool quiet, const std::string &path)
    : Command(force, quiet), lc_(logger_, force, path.empty() ? getProjectRoot() : getProjectRoot(path))
{
}

int Clean::operator()()
{
  lc_.cleanProject();
  return 0;
}
