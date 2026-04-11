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
    : targets_(targets), path_(path), global_(global), Command(force, quiet),
      registry_(Registry::getInstance())
{
}

int Install::execute()
{
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

void Install::installFromPath() const
{
  if (!fs::exists(path_))
    throw ZCError(ZC_NOT_FOUND, "The directory " + path_.string() + " does not exist");

  fs::path project_root = getProjectRoot(path_);
  registry_.installPackage(project_root, force_, quiet_);
}
