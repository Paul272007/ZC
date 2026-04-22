#pragma once

#include <filesystem>

#include "helpers.hh"
#include "objects/Controller.hh"
#include "objects/LocalConfig.hh"

class LocalController : public Controller
{
public:
  LocalController(Logger log, bool force, const std::filesystem::path &root = getProjectRoot());
  ~LocalController() = default;

  void cleanProject();
  void buildProject(bool quiet);
  void publishProject();
  void addDependency(Package &p);

  LocalConfig *lc_;

private:
  void generateCMakeLists() const;
  void checkFolderNames() const;

  std::filesystem::path build_dir_ = root_dir_ / BUILD_DIR;
};
