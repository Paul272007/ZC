#pragma once

#include "commands/Command.hh"
#include "objects/Controllers/GlobalController.hh"

class List : public Command
{
public:
  List(bool force, bool quiet, bool global, bool templates, bool p_templates, bool simple);

  int operator()() override;

private:
  const bool global_;
  const bool templates_;
  const bool p_templates_;
  const bool simple_;
  GlobalController g_;
};
