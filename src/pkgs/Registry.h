#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "config/Conf.h"
#include "helpers.h"
#include "Pkg.h"
#include "ui/Table.h"
#include "Version.h"

namespace zc
{

class Project;
struct LocalTarget;
struct RemoteTarget;

class Registry : public Conf
{
public:
  ~Registry() override;
  Registry(Registry &&)                 = delete;
  Registry &operator=(Registry &&)      = delete;
  Registry(const Registry &)            = delete;
  Registry &operator=(const Registry &) = delete;
  [[nodiscard]] static Registry &get();

  [[nodiscard]] const Pkg &get_pkg(const std::string &name);
  [[nodiscard]] Version get_latest(const std::string &name);

  void install_std(const std::string &name, bool force = false);

  void install_from_server(const RemoteTarget &target, bool force = false, size_t jobs = 1);
  void update_from_server(const RemoteTarget &target, bool force, bool use, size_t jobs = 1);

  Project
  install_from_path(const std::filesystem::path &path, bool force, bool save_path = false, size_t jobs = 1);
  Project update_from_path(
    const std::filesystem::path &path, bool force, bool use, bool save_path = false, size_t jobs = 1
  );
  void uninstall_from_path(const std::filesystem::path &path, bool force);

  void uninstall(const std::string &pkg, bool force);
  void uninstall(const LocalTarget &t, bool force);

  void set_default_version(const std::string &name, const Version &version);
  void set_path(const std::string &name, const std::filesystem::path &path);

  [[nodiscard]] bool is_installed(const std::string &name) const;
  [[nodiscard]] bool is_installed(const std::string &name, const Version &v);

  [[nodiscard]] std::map<std::string, Pkg> pkgs() const;
  [[nodiscard]] Table pkgs_table() const;
  [[nodiscard]] std::vector<std::pair<std::string, std::string>> remote_pkgs() const;
  [[nodiscard]] Table remote_pkgs_table() const;

protected:
  void load() override;
  void write() override;

private:
  const std::filesystem::path tmp_dir_;
  const std::filesystem::path cache_dir_;
  const std::filesystem::path bin_links_dir_;
  const std::filesystem::path lib_links_dir_;
  const std::filesystem::path include_links_dir_;

  std::map<std::string, Pkg> pkgs_; // pkg name -> pkg declaration

  explicit Registry(const std::filesystem::path &root = zc_root());

  void add_pkg_to_index(const Pkg &pkg);
  void add_version_to_pkg(
    const std::string &name, const Version &version, const std::map<std::string, Version> &deps
  );
  Pkg remove_pkg_from_index(const std::string &name);

  [[nodiscard]] std::map<std::string, Pkg>::iterator get_pkg_it(const std::string &name);

  void finish_install(Project &p, const std::string &origin, size_t jobs = 1);
  void finish_update(Project &p, bool use, size_t jobs = 1);

  void copy_bin(const Project &p) const;
  void copy_libs(const Project &p) const;
  void copy_headers(const Project &p) const;
  void update_symlinks(const Pkg &p) const;

  [[nodiscard]] std::vector<std::pair<std::string, Version>>
  dependents_of(const std::string &target_pkg_name) const;

  [[nodiscard]] std::vector<std::pair<std::string, Version>>
  dependents_of(const std::string &target_pkg_name, const Version &target_version) const;

  /**
   * @brief Remove tmp dir
   */
  void clean() const;

  /**
   * Download archive, verify its hash, extract it and return the root of the project
   */
  [[nodiscard]] std::filesystem::path download_and_extract(const RemoteTarget &target) const;

  static void verify_headers_structure(const Project &p);
  static void verify_archive_hash(const std::filesystem::path &archive, const std::string &expected);
};

} // namespace zc
