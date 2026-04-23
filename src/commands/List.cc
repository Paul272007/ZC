#include <optional>

#include "commands/List.hh"
#include "objects/Controllers/Controller.hh"
#include "objects/Controllers/GlobalController.hh"
#include "objects/Controllers/LocalController.hh"

using namespace std;

List::List(
    const bool force, const bool quiet, const bool global, const bool templates, const bool p_templates
)
    : Command(force, quiet), global_(global), templates_(templates), p_templates_(p_templates),
      g_(logger_, force)
{
}

int List::operator()()
{
  std::optional<Table> t;

  if (templates_)
    t = g_.templatesTable();
  else if (p_templates_)
    t = g_.projectTemplatesTable();
  else if (global_)
    t = g_.packagesTable();
  else
    t = LocalController(logger_, force_).packagesTable();

  if (t->getSize() < 2)
    logger_(LogLevel::INFO, "Nothing to show.");
  else
    t->draw();

  return 0;
}
