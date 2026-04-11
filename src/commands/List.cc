#include <commands/Command.hh>
#include <iostream>

#include <commands/List.hh>

using namespace std;

List::List(const bool force, const bool quiet, const bool std, const bool all)
    : std_(std), all_(all), registry_(Registry::getInstance()), Command(force, quiet)
{
}

int List::execute()
{
  if (std_ || all_)
  {
    Table std_lib = registry_.stdPackagesTable();

    if (std_lib.getSize() < 2)
    {
      if (!quiet_)
        cout << "No standard libraries to show." << endl;
    }
    else
      std_lib.draw();
  }
  if (!std_ || all_)
  {
    Table lib = registry_.packagesTable();

    if (lib.getSize() < 2)
    {
      if (!quiet_)
        cout << "No user libraries to show." << endl;
    }
    else
      lib.draw();
  }

  return 0;
}
