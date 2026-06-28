#pragma once

#include "Command.h"
#include "../pkgs/Registry.h"
#include "../templates/TemplateEngine.h"

namespace zc
{

enum ListType
{
  ZC_LIST_SHOW_PKGS,
  ZC_LIST_SHOW_REMOTE,
  ZC_LIST_SHOW_TEMPLATES,
  ZC_LIST_SHOW_P_TEMPLATES,
};

class List : public Command
{
public:
  List(bool force, bool templates, bool p_templates, bool remote, bool simple);

  void operator()() override;

private:
  Registry &reg_ = Registry::get();
  TemplateEngine &te_ = TemplateEngine::get();

  ListType   type_;
  const bool simple_;
};

} // namespace zc
