#include "commands/Install.hh"
#include "objects/Controllers/Controller.hh"
#include "objects/Controllers/GlobalController.hh"
#include "objects/Controllers/LocalController.hh"
#include "objects/ZCError.hh"

using namespace std;

Install::Install(
    bool force, bool quiet, bool global, const std::string &path, const std::vector<std::string> &targets
)
    : Command(force, quiet), path_(path)
{
  if (!global && !path_.empty() && getProjectRoot(path_) == getProjectRoot())
    throw ZCError(ZC_BAD_COMMAND, "Cannot install library as its own dependency");

  if (global)
    c_ = new GlobalController(logger_, force);
  else
    c_ = new LocalController(logger_, force);

  targets_ = Controller::parsePackages(targets);
}

int Install::operator()()
{
  if (!targets_.empty() && !path_.empty())
    throw ZCError(ZC_INCOMPATIBLE_FLAGS, "Cannot install from remote and from local path at the same time");

  else if (path_.empty() && !targets_.empty())
    c_->installFromServer(targets_, quiet_); // Only path is empty : install targets from server

  else if (targets_.empty() && !path_.empty())
    c_->installFromPath(path_, quiet_); // Only targets is empty : install from path

  else
    c_->installFromJson(quiet_); // Both path and targets empty : install from registry.json

  c_->r_->write();
  delete c_;
  return 0;
}
