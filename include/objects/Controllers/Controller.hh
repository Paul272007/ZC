#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "helpers.hh"
#include "objects/Configs/Config.hh"
#include "objects/Network.hh"
#include "objects/Registries/Registry.hh"
#include "objects/Table.hh"

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
  [[nodiscard]] std::vector<Package> getPackages() const;
  [[nodiscard]] Table remotePackagesTable() const;
  [[nodiscard]] std::vector<std::string> getRemotePackages() const;
  [[nodiscard]] Package getPackage(const std::string &pkg) const;
  [[nodiscard]] static Targets parsePackages(const std::vector<std::string> &targets);

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
  std::unique_ptr<Config> c_;

protected:
  Controller(const Controller &) = delete;
  Controller &operator=(const Controller &) = delete;
  Controller(Logger &log, bool force, const std::filesystem::path &root)
      : log_(log), force_(force), root_dir_(root), net_(tmp_dir_)
  {
  }
  void clean() const;
  void buildAndIndex(LocalController &pc, bool quiet, const std::string &origin, bool isUpdate);

  Logger log_;
  Network net_;
  bool force_;
  std::unique_ptr<Registry> r_;

private:
  void installExecutable(LocalController &pc);
  void installLibrary(LocalController &pc);
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
  [[nodiscard]] std::string
  resolvePackageUrl(const std::string &name, const std::string &version, const nlohmann::json &index);
  void verifyPackageHash(const std::filesystem::path &archive, const std::string &expected);
};
