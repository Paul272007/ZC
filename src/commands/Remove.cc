#include <string>
#include <vector>

#include "commands/Remove.hh"
#include "objects/Controller.hh"
#include "objects/GlobalController.hh"
#include "objects/LocalController.hh"

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
      logger_(LogLevel::WARNING, "The package " + pkg + " was not found");
    else
      logger_(LogLevel::SUCCESS, "Package " + pkg + " removed successfully.");
  }
  c_->r_->write();
  delete c_;
  return 0;
}
