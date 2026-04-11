#pragma once

#include <filesystem>
#include <vector>

#include <commands/Command.hh>
#include <helpers.hh>
#include <objects/File.hh>
#include <objects/ProjectSettings.hh>
#include <objects/Settings.hh>

#define ZC_MODULES "external"

class Build : public Command
{
public:
  Build(const bool force, const bool quiet, bool release_mode);

  int execute() override;

private:
  void scanSources();
  void generateCMakeLists() const;
  void buildPackage() const;

  const std::filesystem::path root_ = getProjectRoot();
  std::vector<File> sources_;
  bool release_mode_;
  ProjectSettings &project_settings_;
  Registry &registry_;
  Settings &settings_;
};
