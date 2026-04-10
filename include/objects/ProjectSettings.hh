#pragma once

#include <filesystem>
#include <string>
#include <tuple>
#include <vector>

#include <helpers.hh>
#include <nlohmann/json.hpp>
#include <objects/Version.hh>

enum ProjectType
{
  LIB,
  BIN,
  UNDEF
};

using dependency = std::tuple<std::string, Version>;
using dependencies = std::vector<dependency>;

class ProjectSettings
{
public:
  static ProjectSettings &getInstance();
  ProjectSettings(
      const std::string &name, const std::string &author, const std::string &sharedLib,
      const std::string &staticLib, const std::string &version, const std::string &src,
      const std::string &include, const ProjectType &type, const dependencies &deps
  );
  void write() const;
  void load();

  /* Getters */
  const std::filesystem::path getConfigFile() const;
  const dependencies getDeps() const;
  const ProjectType getType() const;
  const std::string &getName() const;
  const std::string &getAuthor() const;
  const std::string &getSharedLibName() const;
  const std::string &getStaticLibName() const;
  const std::string &getExecutableName() const;
  Version getVersion() const;
  const std::filesystem::path &getSrcFolder() const;
  const std::filesystem::path &getIncludeFolder() const;

private:
  ProjectSettings();

  const std::filesystem::path config_file_;
  dependencies deps_;
  ProjectType type_;
  std::string name_;
  std::string author_;
  std::string shared_lib_name_;
  std::string static_lib_name_;
  std::string executable_name_;
  std::optional<Version> version_;
  std::filesystem::path src_folder_;
  std::filesystem::path include_folder_;
};
