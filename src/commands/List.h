#pragma once

#include "../pkgs/Registry.h"
#include "../templates/TemplateEngine.h"
#include "Command.h"

namespace zc
{

enum class ListType : uint8_t
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
  // TODO: --project to see project dependencies and this class inherits from ProjectCommand

private:
  Registry       &reg_ = Registry::get();
  TemplateEngine &te_  = TemplateEngine::get();

  ListType   type_;
  const bool simple_;
};

} // namespace zc
