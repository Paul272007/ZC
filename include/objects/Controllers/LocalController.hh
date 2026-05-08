#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "Controller.hh"
#include "helpers.hh"
#include "objects/Configs/GlobalConfig.hh"
#include "objects/Configs/LocalConfig.hh"

class LocalController : public Controller
{
public:
  LocalController(Logger log, bool force, const std::filesystem::path &root = getProjectRoot());
  ~LocalController() = default;

  void cleanProject();
  void buildProject(bool quiet, bool release_mode);
  void buildProject(bool quiet, bool release_mode, const std::vector<std::string> &compile_options);
  void publishProject();
  bool addDependency(const std::string &target);

  LocalConfig *lc_;
  GlobalConfig *gc_;

private:
  void generateCMakeLists(const std::vector<std::string> &compile_options) const;
  void checkFolderNames() const;

  std::filesystem::path build_dir_;
  std::filesystem::path cmakelists_;
  std::unique_ptr<GlobalConfig> gc_ptr_;
};
