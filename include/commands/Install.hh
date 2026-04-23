#pragma once

#include "commands/Command.hh"
#include "objects/Controllers/Controller.hh"

class Install : public Command
{
public:
  Install(
      bool force, bool quiet, bool global, const std::string &path, const std::vector<std::string> &targets
  );

  int operator()() override;

private:
  const std::filesystem::path path_;
  Targets targets_;

  Controller *c_ = nullptr;
};
