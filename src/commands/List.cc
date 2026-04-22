#include "commands/List.hh"
#include "objects/GlobalController.hh"
#include "objects/LocalController.hh"

using namespace std;

List::List(
    const bool force, const bool quiet, const bool global, const bool templates, const bool p_templates
)
    : Command(force, quiet), global_(global), templates_(templates), p_templates_(p_templates),
      l_(logger_, force), g_(logger_, force)
{
}

int List::operator()()
{
  Table t(
      templates_ ? g_.templatesTable()
                 : (p_templates_ ? g_.projectTemplatesTable()
                                 : (global_ ? g_.r_->packagesTable() : l_.r_->packagesTable()))
  );
  if (t.getSize() < 2)
    logger_(LogLevel::INFO, "Nothing to show.");
  else
    t.draw();

  return 0;
}
