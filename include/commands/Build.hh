#pragma once

#include <filesystem>

#include "commands/Command.hh"
#include "objects/Controllers/GlobalController.hh"
#include "objects/Controllers/LocalController.hh"

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
