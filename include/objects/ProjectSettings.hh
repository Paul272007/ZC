#pragma once

#include <filesystem>
#include <string>

#include <nlohmann/json.hpp>
#include <objects/Settings.hh>
#include <objects/Version.hh>

enum ProjectType
{
  LIB,
  BIN,
  UNDEF
};

class ProjectSettings : public Settings
{
public:
  explicit ProjectSettings(const std::filesystem::path &project_root);
  ProjectSettings(
      const std::string &name, const std::string &author, const std::string &targetName,
      const std::string &version, const std::string &src, const std::string &include, const ProjectType &type
  );

  void write() const override;

  ProjectType type_;
  std::string name_;
  std::string author_;
  std::string target_name_;
  std::optional<Version> version_;
  std::filesystem::path src_folder_;
  std::filesystem::path include_folder_;

protected:
  void load() override;

private:
  void checkFolderNames() const;
};
