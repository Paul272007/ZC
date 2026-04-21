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
  const bool clean_;
  const bool is_zc_build_command_;

public:
  Build(
      bool force, bool quiet, bool clean, const std::filesystem::path &project_root,
      bool is_zc_build_command = true
  );

  int operator()() override;
  const ProjectSettings p_settings_;
  const Registry registry_;
};
