#include <commands/Command.hh>

#include <commands/List.hh>

using namespace std;
// TODO add --global flag instead of true by default
List::List(const bool force, const bool quiet) : Command(force, quiet), registry_(Registry(true))
{
}

int List::operator()()
{
  if (Table lib = registry_.packagesTable(); lib.getSize() < 2)
    log_info("No user libraries to show.");
  else
    lib.draw();

  return 0;
}
