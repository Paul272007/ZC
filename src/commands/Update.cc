#include <filesystem>
#include <memory>

#include "commands/Command.hh"
#include "commands/Update.hh"
#include "helpers.hh"
#include "objects/Controllers/GlobalController.hh"
#include "objects/Controllers/LocalController.hh"
#include "objects/ZCError.hh"

Update::Update(
    bool force, bool quiet, bool global, const std::string &path, std::vector<std::string> &targets
)
    : Command(force, quiet), path_(path)
{
  if (!targets.empty() && !path_.empty())
    throw ZCError(ZC_INCOMPATIBLE_FLAGS, "Cannot update from remote and from local path at the same time");

  if (!global && !path_.empty() && getProjectRoot(path_) == getProjectRoot())
    throw ZCError(ZC_BAD_COMMAND, "Cannot update library as its own dependency");

  if (global)
    c_ = make_unique<GlobalController>(logger_, force);
  else
    c_ = make_unique<LocalController>(logger_, force);

  if (path_.empty())
  {
    removeDuplicates(targets);
    targets_ = Controller::parsePackages(targets);
  }
}

int Update::operator()()
{
  if (path_.empty())
  {
    if (targets_.empty())
      c_->updateFromJson(quiet_); // Both path and targets empty : install from registry.json
    else
      c_->updateFromServer(targets_, quiet_); // Only path is empty : update targets from server
  }
  else
    c_->updateFromPath(getProjectRoot(path_), quiet_); // Only targets is empty : update from path

  c_->saveRegistry();
  return 0;
}
