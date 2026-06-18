#include <iostream>
#include <vector>

#include "commands/Command.h"
#include "commands/List.h"
#include "helpers.h"
#include "pkgs/Pkg.h"
#include "pkgs/Registry.h"
#include "templates/TemplateEngine.h"
#include "ui/Table.h"

ZC_DEV_CONFIG

namespace zc
{

List::List(bool force, bool templates, bool p_templates, bool remote, bool simple)
    : Command(force), simple_(simple)
{
  type_ = parse_mode<ListType>(
      {{ZC_LIST_SHOW_REMOTE, remote},
       {ZC_LIST_SHOW_TEMPLATES, templates},
       {ZC_LIST_SHOW_P_TEMPLATES, p_templates}},
      ZC_LIST_SHOW_PKGS, "Cannot show multiple things at the same time"
  );
}

int List::operator()()
{
  Registry &reg(Registry::get());
  TemplateEngine &te(TemplateEngine::get());

  if (simple_)
  {
    vector<string> v;

    switch (type_)
    {
    case ZC_LIST_SHOW_PKGS:
      for (const auto &p : reg.pkgs()) v.emplace_back(p.name);
      break;

    case ZC_LIST_SHOW_REMOTE:
      v = reg.remote_pkgs();
      break;

    case ZC_LIST_SHOW_TEMPLATES:
      for (const auto &t : te.templates()) v.emplace_back(t.filename());
      break;

    case ZC_LIST_SHOW_P_TEMPLATES:
    default:
      for (const auto &pt : te.p_templates()) v.emplace_back(pt.filename());
      break;
    }
    for (const auto &elt : v) cout << elt << endl;
  }
  else
  {
    Table t;

    switch (type_)
    {
    case ZC_LIST_SHOW_PKGS:
      t = reg.pkgs_table();
      break;
    case ZC_LIST_SHOW_REMOTE:
      t = reg.remote_pkgs_table();
      break;
    case ZC_LIST_SHOW_TEMPLATES:
      t = te.templates_table();
      break;
    case ZC_LIST_SHOW_P_TEMPLATES:
    default:
      t = te.p_templates_table();
      break;
    }
    t.draw();
  }
  return 0;
}

} // namespace zc
