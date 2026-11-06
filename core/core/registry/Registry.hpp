#pragma once

#include <expected>
#include <string_view>

#include "core/Conf.hpp"
#include "core/registry/RegistryPkg.hpp"
#include "core/registry/RegistryProject.hpp"
#include "framework/Exception.hpp"
#include "framework/util/types.hpp"
#include "framework/Version.hpp"

namespace zc::core
{

class Project;

using PkgMap = std::map<std::string, RegistryPkg, std::less<>>;

class Registry final : public Conf
{
  /// @brief All installed packages (pkg name -> declaration)
  PkgMap pkgs_;

  explicit Registry();

  // ——— Index ———

  /// @brief Add a newly installed package to the index
  /// @param pkg The package to be added
  RegistryPkg *addToIndex(RegistryPkg pkg);

  /// @brief Remove a package from the index
  /// @param name The name of the package to remove
  /// @return The removed package or an exception if something went wrong
  std::expected<RegistryPkg, Exception> removeFromIndex(std::string_view name);

  /// @brief Remove a version of a package
  /// @param name The name of the package to remove
  /// @param version The version of this package we want to remove
  /// @return The removed package snapshot
  std::expected<RegistryProject, Exception> removeFromIndex(std::string_view name, Version version);

  std::expected<RegistryPkg *, Exception> addVersion(const PkgMap::iterator &pkg, RegistryProject snapshot);

  // ——— Private utilities ———

  [[nodiscard]] std::expected<PkgMap::iterator, Exception> getPkgIt(std::string_view name);

  /// @brief Remove temporary directory and its contents
  static void clean();

  std::expected<RegistryPkg *, Exception>
  finishInstallation(Project &p, const std::string &origin, u32 jobs);

  std::expected<RegistryPkg *, Exception>
  finishUpdate(Project &project, const PkgMap::iterator &pkg, u32 jobs, bool set_as_default = true);

  [[nodiscard]] std::map<std::string, Version, std::less<>> dependentsOf(std::string_view name);

  [[nodiscard]] std::map<std::string, Version, std::less<>>
  dependentsOf(std::string_view name, Version version);

protected:
  // ——— I/O ———

  void load() override;
  void write() override;

public:
  ~Registry() override;
  Registry(Registry &&)                 = delete;
  Registry(const Registry &)            = delete;
  Registry &operator=(Registry &&)      = delete;
  Registry &operator=(const Registry &) = delete;

  [[nodiscard]] static Registry &get();

  // ——— Getter ———

  [[nodiscard]] const PkgMap &pkgs() const noexcept { return pkgs_; }

  // ——— Public utilities ———

  [[nodiscard]] const RegistryPkg     *getPkg(std::string_view name) const;
  [[nodiscard]] const RegistryProject *getSnapshot(std::string_view name, Version version) const;

  [[nodiscard]] std::expected<RegistryProject *, Exception> defaultOf(std::string_view name);
  [[nodiscard]] std::expected<Version, Exception>           latestOf(std::string_view name) const;

  [[nodiscard]] std::expected<void, Exception> setPath(std::string_view name, const stdfs::path &path);
  [[nodiscard]] std::expected<void, Exception> setDefaultVersion(std::string_view name, Version version);

  [[nodiscard]] bool isInstalled(std::string_view name) const;
  [[nodiscard]] bool isInstalled(std::string_view name, Version version) const;

  // ——— Installation / Update / Uninstallation methods ———

  [[nodiscard]] std::expected<RegistryPkg *, Exception>
  installStd(std::string name, std::vector<std::unique_ptr<RegistryTarget>> targets);

  [[nodiscard]] std::expected<RegistryPkg *, Exception>
  installFromServer(std::string_view name, Version version, u32 jobs, bool force = false);

  [[nodiscard]] std::expected<RegistryPkg *, Exception> updateFromServer(
    std::string_view name, Version version, u32 jobs, bool force = false, bool set_as_default = true
  );

  [[nodiscard]] std::expected<RegistryPkg *, Exception>
  installFromPath(Project &p, u32 jobs, bool save_path, bool force = false);

  [[nodiscard]] std::expected<RegistryPkg *, Exception>
  updateFromPath(Project &p, u32 jobs, bool save_path, bool force = false, bool set_as_default = true);

  [[nodiscard]] std::expected<RegistryProject, Exception> uninstallFromPath(const Project &p, bool force);

  [[nodiscard]] std::expected<RegistryPkg, Exception> uninstall(std::string_view name, bool force);
  [[nodiscard]] std::expected<RegistryProject, Exception>
  uninstall(std::string_view name, Version version, bool force);
};

inline Registry &reg()
{
  return Registry::get();
}

} // namespace zc::core
