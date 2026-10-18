#pragma once

#include "core/Conf.hpp"
#include "core/project/BuildMode.hpp"
#include "core/project/target/ProjectTarget.hpp"
#include "framework/Version.hpp"
#include "Language.hpp"
#include "LanguageConf.hpp"
#include "target/ProjectTargetDependency.hpp"

namespace zc::core
{

class Project final : public Conf
{
  // Stays const because it is used to initialize static variable in buildDir()
  const stdfs::path root_;

  std::string name_;
  std::string author_;
  std::string description_;

  Version version_;

  std::map<Language, LanguageConf>            languages_;
  std::vector<std::unique_ptr<ProjectTarget>> targets_;

  [[nodiscard]] const stdfs::path &buildDir() const;

  /// @brief Check if the project's structure is correct
  /// @return Whether the project structure is valid or not
  [[nodiscard]] bool isStructureValid() const;

protected:
  void load() override;
  void write() override;

public:
  explicit Project(stdfs::path root = stdfs::current_path());

  // ——— Getters ———

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

  // ——— Build ———

  std::expected<void, Exception>
  buildTarget(std::string_view target, BuildMode mode = BuildMode::Auto, u32 jobs = 1);

  std::expected<void, Exception> buildAll(BuildMode mode = BuildMode::Auto, u32 jobs = 1);

  std::expected<void, Exception> installBuild(u32 jobs = 1);

  // ——— Targets installation ———

  std::expected<void, Exception> installTargets();
};

} // namespace zc::core

// zc
