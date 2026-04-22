#pragma once

#include "commands/Command.hh"
#include "objects/LocalController.hh"

class Clean : public Command
{
public:
  Clean(bool force, bool quiet);

  int operator()() override;

private:
  LocalController lc_;
};
