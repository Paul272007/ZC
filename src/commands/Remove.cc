#include "objects/ProjectSettings.hh"
#include <string>
#include <vector>

#include <commands/Remove.hh>
#include <interface.hh>
#include <objects/Registry.hh>

Remove::Remove(const std::vector<std::string> &targets, const bool force, const bool quiet, const bool global)
    : targets_(targets), Command(force, quiet), global_(global)
{
}

int Remove::execute()
{
  if (global_)
  {
    registry_ = &Registry::getInstance();
    for (const auto &pkg : targets_)
    {
      if (!registry_->removePackage(pkg) && !quiet_)
        warning("All headers / binaries for package " + pkg + " weren't deleted successfully.");
      else
        success("Package " + pkg + " removed successfully.");
    }
  }
  else
  {
    p_settings_ = &ProjectSettings::getInstance();
    for (const auto pkg : targets_)
    {
    }
  }
  return 0;
}
