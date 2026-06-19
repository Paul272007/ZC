#pragma once

#include "Command.h"

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
  ListType type_;
  const bool simple_;
};

} // namespace zc
