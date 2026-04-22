#pragma once

#include <string>
#include <vector>

#include "commands/Command.hh"
#include "objects/GlobalController.hh"
#include "objects/LocalController.hh"

class Add : public Command
{
public:
  Add(const bool force, const bool quiet, const std::vector<std::string> &targets);

  int operator()() override;

private:
  const std::vector<std::string> targets_;
  LocalController l_;
  GlobalController g_;
};
