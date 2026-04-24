#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "helpers.hh"
#include "objects/Configs/Config.hh"
#include "objects/Registries/Registry.hh"
#include "objects/Table.hh"

// clang-format off
#define ROOT_DIR                        ".zc"
#define INDEX                    "index.json"
#define CONFIG                      "zc.json"
#define REGISTRY              "registry.json"
#define CMAKELISTS           "CMakeLists.txt"
#define EXTERNAL                   "external"
#define INCLUDE_DIR                 "include"
#define LIB_DIR                         "lib"
#define BIN_DIR                         "bin"
#define BUILD_DIR                     "build"
#define TMP_DIR                         "tmp"
#define TEMPLATES                 "templates"
#define PROJECT_TEMPLATES "project_templates"
#define GH_REPO      "Paul272007/ZC-Registry"
#define INDEX_URL    "https://paul272007.github.io/ZC-Registry/index.json"
// clang-format on

enum class LogLevel
{
  INFO,
  SUCCESS,
  WARNING,
  ERROR,
  DEBUG
};

using Logger = std::function<void(LogLevel, const std::string &)>;
using Targets = std::vector<std::pair<std::string, std::string>>;
using Visited = std::unordered_set<std::string>;
class LocalController;

class Controller
{
public:
  virtual ~Controller();
  void saveRegistry();
  bool removePackage(const std::string &pkg_name);
  [[nodiscard]] bool isInstalled(const std::string &pkg);
  [[nodiscard]] Table packagesTable() const;
  static Targets parsePackages(const std::vector<std::string> &targets);

  void installFromJson(bool quiet);
  void installFromJson(bool quiet, Visited &visited);

  void updateFromJson(bool quiet);
  void updateFromJson(bool quiet, Visited &visited);

  void installFromPath(const std::filesystem::path &root, bool quiet);

  void updateFromPath(const std::filesystem::path &root, bool quiet);

  void installFromServer(Targets &targets, bool quiet);
  void installFromServer(Targets &targets, bool quiet, Visited &visited);

  void updateFromServer(Targets &targets, bool quiet, nlohmann::json &index, Visited &visited);
  void updateFromServer(Targets &targets, bool quiet);

  std::filesystem::path root_dir_;
  std::filesystem::path bin_dir_;
  std::filesystem::path lib_dir_;
  std::filesystem::path include_dir_;
  std::filesystem::path tmp_dir_ = getZCRootDir() / TMP_DIR;
  Config *c_ = nullptr;
  Registry *r_ = nullptr;

protected:
  Controller(Logger &log, bool force, const std::filesystem::path &root)
      : log_(log), force_(force), root_dir_(root)
  {
  }
  Controller(const Controller &) = delete;
  Controller &operator=(const Controller &) = delete;

  void clean();
  void buildAndIndex(LocalController &pc, bool quiet, const std::string &origin, bool isUpdate);

  Logger log_;
  bool force_;

private:
  void installExecutable(LocalController &pc);
  void installLibrary(LocalController &pc);
  void downloadIndex();
  void downloadArchive(const std::string &url, const std::filesystem::path &path);
  void installPackageFromServer(
      const std::string &name, const std::string &version, const nlohmann::json &index, bool quiet, Visited &v
  );
  void updatePackageFromServer(
      const std::string &name, const std::string &version, const nlohmann::json &index, bool quiet, Visited &v
  );
  void extractAndInstall(
      const std::filesystem::path &archive, const std::filesystem::path &dest, bool quiet, Visited &v,
      bool isUpdate
  );
  std::string
  resolvePackageUrl(const std::string &name, const std::string &version, const nlohmann::json &index);
  void verifyPackageHash(const std::filesystem::path &archive, const std::string &expected);
};
