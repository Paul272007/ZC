#pragma once

#include "commands/Command.hh"
#include "objects/Controllers/LocalController.hh"

class Publish : public Command
{
public:
  Publish(const bool force, const bool quiet, const std::string &path);
  int operator()() override;

private:
  LocalController l_;
};
