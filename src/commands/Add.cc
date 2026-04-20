#include <filesystem>
#include <vector>

#include <commands/Add.hh>
#include <helpers.hh>
#include <interface.hh>
#include <objects/Registry.hh>
#include <objects/ZCError.hh>

Add::Add(const std::vector<std::string> &targets, const bool force, const bool quiet)
    : Command(force, quiet), targets_(targets), global_(Registry(true)), local_(Registry(false))
{
}

int Add::operator()()
{
  std::vector<Package> v;
  for (const auto &target : targets_)
  {
    // If the package is in the local registry
    if (local_.pkgExists(target) && !force_)
    {
      // If it is a local dependency
      if (std::filesystem::exists(getProjectRoot() / EXTERNAL / LIB_DIR / target))
      {
        if (ask("The package " + target +
                " is already installed on this project. Do you want to overwrite it ?"))
        {
          std::filesystem::remove_all(getProjectRoot() / EXTERNAL / INCLUDE_DIR / target);
          std::filesystem::remove_all(getProjectRoot() / EXTERNAL / LIB_DIR / target);
          v.push_back(global_.getPackage(target));
        }
      }
      else // If it is a global dependency, just say it was already added
      {
        info("Dependency already added.");
      }
    } // Add the package to the list of packages to index and check if it is globally installed
    v.push_back(global_.getPackage(target));
  }
  bool modifs = false;
  for (const auto &pkg : v)
  {
    local_.indexPackage(pkg);
    modifs = true;
    log_success("Added dependency: " + pkg.name_);
  }
  if (modifs)
    local_.write();
  return 0;
}
