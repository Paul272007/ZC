#include <filesystem>
#include <memory>

#include "commands/Command.hh"
#include "commands/Update.hh"
#include "objects/Controllers/GlobalController.hh"
#include "objects/Controllers/LocalController.hh"
#include "objects/ZCError.hh"

Update::Update(
    bool force, bool quiet, bool global, const std::string &path, const std::vector<std::string> &targets
)
    : Command(force, quiet), path_(path)
{
  if (targets.size() > 1 && !path_.empty())
    throw ZCError(ZC_BAD_COMMAND, "Cannot update two libraries with the same path");

  if (global)
    c_ = make_unique<GlobalController>(logger_, force);
  else
    c_ = make_unique<LocalController>(logger_, force);

  targets_ = Controller::parsePackages(targets);
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
    c_->updateFromPath(path_, quiet_); // Only targets is empty : update from path

  c_->saveRegistry();
  return 0;
}
