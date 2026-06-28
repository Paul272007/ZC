#include "commands/List.h"

#include <iostream>
#include <vector>

#include "commands/Command.h"
#include "helpers.h"
#include "pkgs/Registry.h"
#include "templates/TemplateEngine.h"
#include "ui/Table.h"

ZC_DEV_CONFIG

namespace zc
{

List::List(const bool force, bool templates, bool p_templates, bool remote, const bool simple)
  : Command(force), simple_(simple)
{
  type_ = parse_mode<ListType>(
    {
      { ZC_LIST_SHOW_REMOTE, remote },
      { ZC_LIST_SHOW_TEMPLATES, templates },
      { ZC_LIST_SHOW_P_TEMPLATES, p_templates },
    },
    ZC_LIST_SHOW_PKGS, "Cannot show multiple things at the same time"
  );
}

void List::operator()()
{
  if (simple_)
  {
    vector<string> v;

    switch (type_)
    {
    case ZC_LIST_SHOW_PKGS:
      for (const auto &[name, _] : reg_.pkgs())
        v.emplace_back(name);
      break;

    case ZC_LIST_SHOW_REMOTE:
      for (const auto &[name, _] : reg_.remote_pkgs())
        v.emplace_back(name);
      break;

    case ZC_LIST_SHOW_TEMPLATES:
      v = te_.templates();
      break;

    case ZC_LIST_SHOW_P_TEMPLATES:
    default:
      v = te_.p_templates();
      break;
    }
    for (const auto &elt : v)
      cout << elt << endl;
  }
  else
  {
    Table t;

    switch (type_)
    {
    case ZC_LIST_SHOW_PKGS:
      t = reg_.pkgs_table();
      break;
    case ZC_LIST_SHOW_REMOTE:
      t = reg_.remote_pkgs_table();
      break;
    case ZC_LIST_SHOW_TEMPLATES:
      t = te_.templates_table();
      break;
    case ZC_LIST_SHOW_P_TEMPLATES:
    default:
      t = te_.p_templates_table();
      break;
    }
    t.draw();
  }
}

} // namespace zc
