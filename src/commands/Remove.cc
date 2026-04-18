#include <string>
#include <vector>

#include <commands/Remove.hh>
#include <interface.hh>
#include <objects/Registry.hh>

Remove::Remove(const std::vector<std::string> &targets, const bool force, const bool quiet, const bool global)
    : Command(force, quiet), registry_(Registry(global)), targets_(targets)
{
}

int Remove::operator()()
{
  for (const auto &pkg : targets_)
  {
    if (!registry_.removePackage(pkg))
    {
      if (!quiet_)
        warning("The package " + pkg + " was not found");
    }
    else
    {
      if (!quiet_)
        success("Package " + pkg + " removed successfully.");
    }
  }
  registry_.write();
  return 0;
}
