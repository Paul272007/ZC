#include "commands/Publish.hh"
#include "helpers.hh"
#include "objects/Controllers/LocalController.hh"

using namespace std;
using json = nlohmann::json;

Publish::Publish(const bool force, const bool quiet, const std::string &path)
    : Command(force, quiet), l_(logger_, force, path.empty() ? getProjectRoot() : getProjectRoot(path))
{
}

int Publish::operator()()
{
  l_.publishProject();
  return 0;
}
