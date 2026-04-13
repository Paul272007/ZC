#pragma once

#include <filesystem>
#include <string>

#include <nlohmann/json.hpp>
#include <objects/Version.hh>

enum ProjectType
{
  LIB,
  BIN,
  UNDEF
};

class ProjectSettings
{
public:
  static ProjectSettings &getInstance(const std::filesystem::path &project_root);
  ProjectSettings(
      const std::string &name, const std::string &author, const std::string &sharedLib,
      const std::string &staticLib, const std::string &version, const std::string &src,
      const std::string &include, const ProjectType &type
  );
  void write() const;
  void load();

  const std::filesystem::path project_root_;
  const std::filesystem::path config_file_;
  ProjectType type_;
  std::string name_;
  std::string author_;
  std::string shared_lib_name_;
  std::string static_lib_name_;
  std::string executable_name_;
  std::optional<Version> version_;
  std::filesystem::path src_folder_;
  std::filesystem::path include_folder_;
private:
  explicit ProjectSettings(const std::filesystem::path &project_root);
};
