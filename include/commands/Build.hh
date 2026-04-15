#pragma once

#include <filesystem>
#include <vector>

#include <commands/Command.hh>
#include <objects/File.hh>
#include <objects/ProjectSettings.hh>
#include <objects/Registry.hh>

class Build : public Command
{
private:
  /**
   * @brief Generate CMakeLists.txt based on the sources and the dependencies
   */
  void generateCMakeLists() const;

  const std::filesystem::path root_;
  std::vector<File> sources_;

public:
  Build(bool force, bool quiet, const std::filesystem::path &project_root);

  int operator()() override;
  const ProjectSettings &p_settings_;
  const Registry registry_;
};
