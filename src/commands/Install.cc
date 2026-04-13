#include <filesystem>

#include <commands/Install.hh>
#include <helpers.hh>
#include <objects/Registry.hh>
#include <objects/ZCError.hh>

using namespace std;
namespace fs = std::filesystem;

Install::Install(
    const std::vector<std::string> &targets, const std::string &path, const bool global, const bool force,
    const bool quiet
)
    : Command(force, quiet), targets_(targets), path_(path), registry_(Registry(global))
{
  if (!global && getProjectRoot(path_) == getProjectRoot())
    throw ZCError(ZC_BAD_COMMAND, "Cannot install library as its own dependency");
}

int Install::operator()()
{
  if (!targets_.empty() && !path_.empty())
    throw ZCError(ZC_INCOMPATIBLE_FLAGS, "Cannot install from remote and from local path at the same time");

  if (targets_.empty() && path_.empty())
    throw ZCError(); // Both path and targets empty : install from zc.json

  if (path_.empty())
    throw ZCError(); // Only path is empty : install targets from server

  if (targets_.empty())
    installFromPath(); // Only targets is empty : install from path

  registry_.write();
  return 0;
}

void Install::installFromJson() const
{
}

void Install::installFromPath()
{
  if (!fs::exists(path_))
    throw ZCError(ZC_NOT_FOUND, "The directory " + path_.string() + " does not exist");

  registry_.installPackage(getProjectRoot(path_), force_, quiet_);
}
