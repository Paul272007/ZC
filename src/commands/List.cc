#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "commands/List.hh"
#include "helpers.hh"
#include "objects/Controllers/Controller.hh"
#include "objects/Controllers/GlobalController.hh"
#include "objects/Controllers/LocalController.hh"

using namespace std;

List::List(
    const bool force, const bool quiet, const bool global, const bool templates, const bool p_templates,
    const bool simple, const bool remote, const std::string &path
)
    : Command(force, quiet), global_(global), templates_(templates), p_templates_(p_templates),
      simple_(simple), remote_(remote), g_(logger_, force), path_(path)
{
}

int List::operator()()
{
  if (simple_)
  {
    std::vector<std::string> v;

    if (templates_)
      for (const auto &f : g_.getTemplates()) v.emplace_back(f.filename().string());
    else if (p_templates_)
      for (const auto &f : g_.getProjectTemplates()) v.emplace_back(f.filename().string());
    else if (remote_)
      v = g_.getRemotePackages();
    else if (global_)
      for (const auto &p : g_.getPackages()) v.emplace_back(p.name);
    else
      for (const auto &p :
           LocalController(logger_, force_, path_.empty() ? getProjectRoot() : getProjectRoot(path_))
               .getPackages())
        v.emplace_back(p.name);

    for (const auto &elt : v) cout << elt << endl;
  }
  else
  {
    std::optional<Table> t;

    if (templates_)
      t = g_.templatesTable();
    else if (p_templates_)
      t = g_.projectTemplatesTable();
    else if (remote_)
      t = g_.remotePackagesTable();
    else if (global_)
      t = g_.packagesTable();
    else
      t = LocalController(logger_, force_, path_.empty() ? getProjectRoot() : getProjectRoot(path_))
              .packagesTable();

    if (t->getSize() < 2)
      logger_(LogLevel::INFO, "Nothing to show.");
    else
      t->draw();
  }

  return 0;
}
