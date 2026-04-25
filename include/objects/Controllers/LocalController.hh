#pragma once

#include <filesystem>

#include "Controller.hh"
#include "helpers.hh"
#include "objects/Configs/LocalConfig.hh"

class LocalController : public Controller
{
public:
  LocalController(Logger log, bool force, const std::filesystem::path &root = getProjectRoot());
  ~LocalController() = default;

  void cleanProject();
  void buildProject(bool quiet, bool release_mode);
  void publishProject();
  bool addDependency(const std::string &target);

  LocalConfig *lc_;

private:
  void generateCMakeLists() const;
  void checkFolderNames() const;

  std::filesystem::path build_dir_;
  std::filesystem::path cmakelists_;
};
