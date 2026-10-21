#pragma once

#include "core/Conf.hpp"
#include "core/project/BuildMode.hpp"
#include "core/project/LanguageConf.hpp"
#include "core/project/target/ProjectTarget.hpp"
#include "framework/Version.hpp"

namespace zc::core
{

class Project final : public Conf
{
  const stdfs::path root_;

  std::string name_;
  std::string author_;
  std::string description_;

  Version version_;

  std::map<Language, LanguageConf>            languages_;
  std::vector<std::unique_ptr<ProjectTarget>> targets_;

  /// @brief Check if the project's structure is correct
  /// @return Whether the project structure is valid or not
  [[nodiscard]] bool isStructureValid() const;

  [[nodiscard]] BuildMode currentBuildMode() const;

  [[nodiscard]] std::expected<void, Exception> generateBuildFile(BuildMode mode);
  [[nodiscard]] std::expected<void, Exception> generateCompileCommands(BuildMode mode);

protected:
  void load() override;
  void write() override;

public:
  // Because of the parent pointers in the targets
  Project(const Project &)            = delete;
  Project(Project &&)                 = delete;
  Project &operator=(const Project &) = delete;
  Project &operator=(Project &&)      = delete;
  ~Project() override                 = default;

  explicit Project(stdfs::path root = stdfs::current_path());

  // ——— Getters ———

  [[nodiscard]] stdfs::path buildDir() const { return root_ / zc_build_dir; }

  [[nodiscard]] stdfs::path cacheDir() const { return root_ / zc_project_cache_dir; }

  [[nodiscard]] stdfs::path ninjaFile() const { return buildDir() / zc_ninja_file; }

  [[nodiscard]] stdfs::path buildModeFile() const { return buildDir() / zc_build_mode_file; }

  [[nodiscard]] const stdfs::path &root() const noexcept { return root_; }

  [[nodiscard]] const std::string &name() const noexcept { return name_; }

  [[nodiscard]] const std::string &author() const noexcept { return author_; }

  [[nodiscard]] const std::string &description() const noexcept { return description_; }

  [[nodiscard]] Version version() const noexcept { return version_; }

  [[nodiscard]] const std::map<Language, LanguageConf> &languages() const noexcept { return languages_; }

  [[nodiscard]] const std::vector<std::unique_ptr<ProjectTarget>> &targets() const noexcept
  {
    return targets_;
  }

  // ——— Dependencies ———

  std::expected<void, Exception> installDependencies();

  std::expected<void, Exception> updateDependencies();

  // ——— Public utilities ———

  void clean() const;

  // ——— Build ———

  std::expected<void, Exception> generateBuildConfig(BuildMode mode);

  std::expected<void, Exception> generateBuildConfigInstall(BuildMode mode);

  std::expected<void, Exception>
  buildTarget(std::string_view target, BuildMode mode = BuildMode::Auto, u32 jobs = 1);

  std::expected<void, Exception> buildAll(BuildMode mode = BuildMode::Auto, u32 jobs = 1);

  std::expected<void, Exception> installBuild(u32 jobs = 1);

  // ——— Targets installation ———

  std::expected<void, Exception> installTargets();
};

} // namespace zc::core

// zc
