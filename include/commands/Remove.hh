#pragma once

#include <string>
#include <vector>

#include "commands/Command.hh"

class Remove : public Command
{
public:
  Remove(bool force, bool quiet, bool global, const std::vector<std::string> &targets);

  int operator()() override;

private:
  std::vector<std::string> targets_;
  Controller *c_ = nullptr;
};
