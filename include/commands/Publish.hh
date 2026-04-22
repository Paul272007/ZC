#pragma once

#include "commands/Command.hh"
#include "objects/LocalController.hh"

class Publish : public Command
{
public:
  Publish(const bool force, const bool quiet);
  int operator()() override;

private:
  LocalController l_;
};
