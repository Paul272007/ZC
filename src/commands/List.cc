#include "commands/Command.hh"
#include <iostream>

#include <commands/List.hh>

using namespace std;

List::List(const bool force, const bool quiet)
    : p_registry_(ProjectsRegistry::getInstance()), Command(force, quiet)
{
}

int List::execute()
{
  Table projects = p_registry_.projectsTable();

  if (projects.getSize() < 2)
  {
    if (!quiet_)
      cout << "No projects to show." << endl;
    return 0;
  }

  projects.draw();
  return 0;
}