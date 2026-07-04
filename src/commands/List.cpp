#include "commands/List.h"

#include <iostream>
#include <ranges>
#include <vector>

#include "helpers.h"
#include "pkgs/Registry.h"
#include "templates/TemplateEngine.h"
#include "ui/Table.h"

ZC_DEV_CONFIG

namespace zc
{

List::List(const CommandContext &ctx, bool deps, bool templates, bool p_templates, bool remote, bool simple)
  : Command(ctx, deps), simple_(simple)
{
  type_ = parse_mode<ListType>(
    {
      { ListType::SHOW_DEPS, deps },
      { ListType::SHOW_REMOTE, remote },
      { ListType::SHOW_TEMPLATES, templates },
      { ListType::SHOW_P_TEMPLATES, p_templates },
    },
    ListType::SHOW_PKGS, "Cannot show multiple things at the same time"
  );
}

void List::operator()()
{
  if (simple_)
  {
    vector<string> v;

    switch (type_)
    {
    case ListType::SHOW_PKGS:
      for (const auto &[name, _] : reg_.pkgs())
        v.emplace_back(name);
      break;

    case ListType::SHOW_DEPS:
      for (const auto &d : p().pconf.dependencies | views::values)
        v.emplace_back(d.name);
      break;

    case ListType::SHOW_REMOTE:
      for (const auto &[name, _] : reg_.remote_pkgs())
        v.emplace_back(name);
      break;

    case ListType::SHOW_TEMPLATES:
      v = te_.templates();
      break;

    case ListType::SHOW_P_TEMPLATES:
    default:
      v = te_.p_templates();
      break;
    }
    for (const auto &elt : v)
      cout << elt << '\n';
  }
  else
  {
    Table t;

    switch (type_)
    {
    case ListType::SHOW_PKGS:
      t = reg_.pkgs_table();
      break;

    case ListType::SHOW_DEPS:
      t = p().pconf.dependencies_table();
      break;

    case ListType::SHOW_REMOTE:
      t = reg_.remote_pkgs_table();
      break;

    case ListType::SHOW_TEMPLATES:
      t = te_.templates_table();
      break;

    case ListType::SHOW_P_TEMPLATES:
    default:
      t = te_.p_templates_table();
      break;
    }
    t.draw();
  }
}

} // namespace zc
