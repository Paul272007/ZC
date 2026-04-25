#pragma once

#include "commands/Command.hh"
#include <memory>

class Update : public Command
{
public:
  Update(bool force, bool quiet, bool global, const std::string &path, std::vector<std::string> &targets);

  int operator()() override;

private:
  const std::filesystem::path path_;
  Targets targets_;

  std::unique_ptr<Controller> c_ = nullptr;
};
