#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "commands/Command.hh"
#include "objects/Controllers/GlobalController.hh"

class Create : public Command
{
public:
  Create(
      bool force, bool quiet, bool edit, std::vector<std::string> &files,
      const std::vector<std::string> &input_files
  );

  int operator()() override;

private:
  void writeCDecls(const std::filesystem::path &f) const;

  bool edit_;
  std::vector<std::filesystem::path> files_;
  std::vector<std::filesystem::path> input_files_;
  GlobalController g_;
};
