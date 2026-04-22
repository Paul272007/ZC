#pragma once

#include "commands/Command.hh"
#include "objects/GlobalController.hh"
#include "objects/LocalController.hh"

class List : public Command
{
public:
  List(bool force, bool quiet, bool global, bool templates, bool p_templates);

  int operator()() override;

private:
  const bool templates_;
  const bool p_templates_;
  const bool global_;
  LocalController l_;
  GlobalController g_;
};
