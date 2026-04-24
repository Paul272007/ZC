#pragma once

#include <filesystem>
#include <string>

#include "commands/Command.hh"
#include "objects/Configs/LocalConfig.hh"
#include "objects/Controllers/GlobalController.hh"

class Init : public Command
{
public:
  Init(
      bool force, bool quiet, bool edit, bool git, const std::string &path, const std::string &author,
      const std::string &project_template, const std::string &name, const std::string &type
  );

  int operator()() override;

private:
  const bool git_;
  const bool edit_;
  Type type_;
  std::string name_;
  std::string target_;
  std::string author_;
  std::string template_;
  std::filesystem::path path_;
  std::vector<std::string> project_templates_;
  GlobalController g_;
};
