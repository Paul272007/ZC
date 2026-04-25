#pragma once

#include "commands/Command.hh"

class Install : public Command
{
public:
  Install(bool force, bool quiet, bool global, const std::string &path, std::vector<std::string> &targets);

  int operator()() override;

private:
  const std::filesystem::path path_;
  Targets targets_;

  std::unique_ptr<Controller> c_ = nullptr;
};
