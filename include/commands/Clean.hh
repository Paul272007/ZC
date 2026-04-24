#pragma once

#include "commands/Command.hh"
#include "objects/Controllers/LocalController.hh"

class Clean : public Command
{
public:
  Clean(bool force, bool quiet, const std::string &path);

  int operator()() override;

private:
  LocalController lc_;
};
