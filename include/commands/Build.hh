#pragma once

#include <filesystem>

#include "commands/Command.hh"
#include "objects/GlobalController.hh"
#include "objects/LocalController.hh"

class Build : public Command
{
public:
  Build(bool force, bool quiet, bool clean, const std::string &path);

  int operator()() override;

private:
  const bool clean_;
  const std::filesystem::path path_;
  LocalController l_;
  GlobalController g_;
};
