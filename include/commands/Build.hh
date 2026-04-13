#pragma once

#include <filesystem>
#include <vector>

#include <commands/Command.hh>
#include <objects/File.hh>
#include <objects/ProjectSettings.hh>
#include <objects/Registry.hh>

class Build : public Command
{
public:
  Build(bool force, bool quiet, bool release_mode);
  Build(bool force, bool quiet, const std::filesystem::path &project_root);

  int execute() override;
  const ProjectSettings &p_settings_;
  const Registry registry_;

private:
  /**
   * @brief Generate CMakeLists.txt based on the sources and the dependencies
   */
  void generateCMakeLists() const;

  const std::filesystem::path root_;
  bool release_mode_;
  std::vector<File> sources_;
};
