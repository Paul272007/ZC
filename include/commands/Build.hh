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
  Build(const bool force, const bool quiet, const ProjectSettings &project_settings);

  int execute() override;

private:
  /**
   * @brief Scan the source folder and get all C or C++ files found
   */
  void scanSources();

  /**
   * @brief Generate CMakeLists.txt based on the sources and the dependencies
   */
  void generateCMakeLists() const;

  const std::filesystem::path root_ = getProjectRoot();
  std::vector<File> sources_;
  bool release_mode_;
  const ProjectSettings &project_settings_;
  Registry &registry_;
  Settings &settings_;
};
