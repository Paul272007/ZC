#include <commands/Command.hh>
#include <iostream>

#include <commands/List.hh>

using namespace std;
// TODO add --global flag instead of true by default
List::List(const bool force, const bool quiet, const bool std, const bool all)
    : Command(force, quiet), std_(std), all_(all), registry_(Registry(true, true))
{
}

int List::execute()
{
  if (std_ || all_)
  {
    if (Table std_lib = registry_.stdPackagesTable(); std_lib.getSize() < 2)
    {
      if (!quiet_)
        cout << "No standard libraries to show." << endl;
    }
    else
      std_lib.draw();
  }
  if (!std_ || all_)
  {
    if (Table lib = registry_.packagesTable(); lib.getSize() < 2)
    {
      if (!quiet_)
        cout << "No user libraries to show." << endl;
    }
    else
      lib.draw();
  }

  return 0;
}
