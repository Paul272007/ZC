#include "commands/Add.hh"
#include "helpers.hh"

Add::Add(const bool force, const bool quiet, const std::vector<std::string> &targets)
    : Command(force, quiet), targets_(targets), l_(logger_, force)
{
  removeDuplicates(targets_);
}

int Add::operator()()
{
  bool modifs = false;
  for (const auto &target : targets_)
  {
    if (l_.addDependency(target))
      modifs = true;
  }

  if (modifs)
    l_.saveRegistry();

  return 0;
}
