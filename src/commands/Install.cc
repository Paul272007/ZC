#include "objects/ProjectSettings.hh"
#include <filesystem>

#include <commands/Install.hh>
#include <helpers.hh>
#include <interface.hh>
#include <objects/Registry.hh>
#include <objects/ZCError.hh>

using namespace std;
namespace fs = std::filesystem;

Install::Install(
    const std::vector<std::string> &targets, const std::string &path, const bool global, const bool force,
    const bool quiet
)
    : targets_(targets), path_(path), global_(global), Command(force, quiet)
{
}

int Install::execute()
{
  if (global_)
    registry_ = &Registry::getInstance();
  else
    p_settings_ = &ProjectSettings::getInstance();

  if (!targets_.empty() && !path_.empty())
    throw ZCError(ZC_INCOMPATIBLE_FLAGS, "Cannot install from remote and from local path at the same time");

  if (targets_.empty() && path_.empty())
    throw ZCError(); // Both path and targets empty : install from zc.json

  if (path_.empty())
    throw ZCError(); // Only path is empty : install targets from server

  if (targets_.empty())
    installFromPath(); // Only targets is empty : install from path

  return 0;
}

void Install::installFromJson()
{
}

void Install::installFromPath()
{
  if (!fs::exists(path_))
    throw ZCError(ZC_NOT_FOUND, "The directory " + path_.string() + " does not exist");

  fs::path project_root = getProjectRoot(path_);

  if (global_)
    registry_->installPackage(project_root, force_, quiet_);
  else
    p_settings_->installPackage(project_root, force_, quiet_);
}
