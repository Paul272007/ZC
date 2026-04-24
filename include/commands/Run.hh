#pragma once

#include <filesystem>

#include "commands/Command.hh"
#include "objects/Controllers/GlobalController.hh"

enum class Mode
{
  FULL,
  PREPROCESS,
  COMPILE,
  ASSEMBLE
};

class Run : public Command
{
public:
  Run(bool force, bool quiet, bool keep, bool plus, bool preprocess, bool compile, bool assemble,
      bool add_std, bool static_compile, const std::vector<std::string> &files,
      const std::vector<std::string> &args);

  int operator()() override;

private:
  static Mode getMode(bool preprocess, bool compile, bool assemble);

  bool compileAsCpp() const;

  void buildCommand();

  std::vector<std::string> getInclusions() const;

  bool plus_ = false;
  const bool add_std_;
  const bool keep_ = false;
  const bool static_ = false;
  Mode mode_;
  std::string output_name_;
  std::string build_cmd_;
  std::vector<std::string> args_;
  std::vector<std::filesystem::path> files_;
  GlobalController g_;
};
