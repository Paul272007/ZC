#include "commands/Publish.hh"
#include "objects/LocalController.hh"

using namespace std;
using json = nlohmann::json;

Publish::Publish(const bool force, const bool quiet) : Command(force, quiet), l_(logger_, force)
{
}

int Publish::operator()()
{
  l_.publishProject();
  return 0;
}
