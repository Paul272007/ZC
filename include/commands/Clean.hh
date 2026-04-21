#pragma once

#include <filesystem>

#include <commands/Command.hh>
#include <helpers.hh>
#include <objects/ProjectSettings.hh>

class Clean : public Command
{
public:
  Clean(bool force, bool quiet, const std::filesystem::path &project_root = getProjectRoot());
  Clean(
      bool force, bool quiet, const ProjectSettings &p_settings,
      const std::filesystem::path &project_root = getProjectRoot()
  );

  int operator()() override;

private:
  const std::filesystem::path root_;
  const ProjectSettings p_settings_;
};
