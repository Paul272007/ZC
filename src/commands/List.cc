#include <filesystem>

#include <commands/Command.hh>
#include <commands/List.hh>
#include <helpers.hh>

using namespace std;
namespace fs = std::filesystem;

List::List(
    const bool force, const bool quiet, const bool global, const bool templates, const bool p_templates
)
    : Command(force, quiet), registry_(global ? Registry() : Registry(getProjectRoot())),
      templates_(templates), p_templates_(p_templates)
{
}

int List::operator()()
{
  if (templates_)
  {
    for (const auto &entry : fs::directory_iterator(getZCRootDir() / "templates"))
    {
    }
  }
  else if (p_templates_)
  {
  }
  else
  {
    if (Table lib = registry_.packagesTable(); lib.getSize() < 2)
      log_info("No user libraries to show.");
    else
      lib.draw();
  }

  return 0;
}
