#include "commands/Add.hh"
#include "objects/Controller.hh"
#include "objects/LocalController.hh"
#include "objects/Registry.hh"

Add::Add(const bool force, const bool quiet, const std::vector<std::string> &targets)
    : Command(force, quiet), targets_(targets), l_(logger_, force), g_(logger_, force)
{
}

int Add::operator()()
{
  std::vector<Package> v;
  for (const auto &target : targets_)
  {
    if (force_)
    {
      v.push_back(g_.r_->getPackage(target));
      continue;
    }
    else
    {
      if (l_.r_->getPackage(target).is_installed_locally)
      {
        logger_(LogLevel::WARNING, "Skipped package '" + target + "' already installed locally.");
        continue;
      }
      v.push_back(g_.r_->getPackage(target));
    }
  }

  bool modifs = false;
  for (auto &pkg : v)
  {
    l_.addDependency(pkg);
    modifs = true;
  }
  if (modifs)
    l_.r_->write();
  return 0;
}
