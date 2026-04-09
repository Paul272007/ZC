#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <helpers.hh>
#include <interface.hh>
#include <objects/File.hh>
#include <objects/Table.hh>

#define PROJECTS "projects.json"

struct Project
{
  std::string name_;
  std::filesystem::path path_;
  std::string template_;
};

class ProjectsRegistry
{
public:
  static ProjectsRegistry &getInstance();

  Table projectsTable() const;

  void saveProject(const Project &p);

  bool projectExists(const std::string &target);

  void removeProject(const std::string &project_name);

  void purgeProject(const std::string &project_name);

  void writeProjects() const;

private:
  ProjectsRegistry();

  void load();

  std::vector<Project> getProjects_();

  std::vector<Project> projects_;

  std::filesystem::path projects_path_ = getZCRootDir() / PROJECTS;
};