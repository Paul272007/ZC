#include <string>
#include <vector>

#include "commands/Remove.hh"
#include "objects/Controllers/Controller.hh"
#include "objects/Controllers/GlobalController.hh"
#include "objects/Controllers/LocalController.hh"

Remove::Remove(bool force, bool quiet, bool global, const std::vector<std::string> &targets)
    : Command(force, quiet), targets_(targets)
{
  if (global)
    c_ = new GlobalController(logger_, force);
  else
    c_ = new LocalController(logger_, force);
}

int Remove::operator()()
{
  for (const auto &pkg : targets_)
  {
    if (!c_->removePackage(pkg))
      logger_(LogLevel::WARNING, "Skipped package " + pkg + " not found");
    else
      logger_(LogLevel::SUCCESS, "Package " + pkg + " removed successfully.");
  }
  c_->saveRegistry();
  delete c_;
  return 0;
}
