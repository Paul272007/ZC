#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

#include <nlohmann/json.hpp>

#include <objects/ProjectsRegistry.hh>
#include <objects/ZCError.hh>

using namespace std;
using json = nlohmann::json;
namespace fs = std::filesystem;

ProjectsRegistry::ProjectsRegistry()
{
  load();
}

ProjectsRegistry &ProjectsRegistry::getInstance()
{
  static ProjectsRegistry instance;
  return instance;
};

void ProjectsRegistry::load()
{
  json json_projects;
  if (!fs::exists(projects_path_))
  {
    throw ZCError(ZC_CONFIG_NOT_FOUND, "The projects registry was not found: " + projects_path_.string());
  }
  ifstream input(projects_path_);
  if (!input.is_open())
    throw ZCError(
        ZC_CONFIG_READING_ERROR, "The projects registry couldn't be read: " + projects_path_.string()
    );
  try
  {
    input >> json_projects;
  }
  catch (const json::parse_error &e)
  {
    throw ZCError(
        ZC_CONFIG_PARSING_ERROR,
        "The projects registry couldn't be parsed: " + projects_path_.string() + ": " + e.what()
    );
  }
  if (json_projects.contains("projects") && json_projects["projects"].is_object())
  {
    for (auto &[key, value] : json_projects["projects"].items())
    {
      Project p;
      p.name_ = key;

      if (!value.is_object() || !value.contains("path") || !value.contains("template"))
        throw ZCError(ZC_CONFIG_CONTENT_ERROR, "The projects registry is not correctly written");

      p.path_ = value.value("path", "");
      p.template_ = value.value("template", "");

      projects_.push_back(p);
    }
  }
};

void ProjectsRegistry::saveProject(const Project &p)
{
  projects_.push_back(p);
  writeProjects();
};

void ProjectsRegistry::writeProjects() const
{
  json root;
  root["projects"] = json::object();

  for (const auto &[name_, path_, template_] : projects_)
  {
    root["projects"][name_] = json::object();
    root["projects"][name_]["path"] = path_;
    root["projects"][name_]["template"] = template_;
  }
  ofstream output(projects_path_);
  if (!output.is_open())
  {
    throw ZCError(
        ZC_CONFIG_WRITING_ERROR, "The projects registry couldn't be written: " + projects_path_.string()
    );
  }
  output << root.dump(4);
  output.close();
}

Table ProjectsRegistry::projectsTable() const
{
  vector<vector<string>> str_projects{{"Project name", "Path", "Template"}};

  for (const auto &[name_, path_, template_] : projects_)
    str_projects.push_back({name_, path_.string(), template_});

  return Table(str_projects.size(), 3, false, true, str_projects);
};

vector<Project> ProjectsRegistry::getProjects_()
{
  return projects_;
}

bool ProjectsRegistry::projectExists(const std::string &target)
{
  const auto it = ranges::find_if(projects_, [&target](const Project &p) { return p.name_ == target; });

  return it != projects_.end();
}

void ProjectsRegistry::removeProject(const std::string &project_name)
{
  if (const auto it = ranges::find_if(projects_, [&](const Project &p) { return p.name_ == project_name; });
      it != projects_.end())
    projects_.erase(it);
  else
    throw ZCError(
        ZC_PROJECT_NOT_FOUND, "The project was not found in the projects registry: " + project_name
    );

  writeProjects();
}

void ProjectsRegistry::purgeProject(const std::string &project_name)
{
  if (const auto it = ranges::find_if(projects_, [&](const Project &p) { return p.name_ == project_name; });
      it != projects_.end())
  {

    if (const fs::path path(it->path_); fs::exists(path))
      fs::remove_all(path);

    projects_.erase(it);
  }
  else
    throw new ZCError(
        ZC_PROJECT_NOT_FOUND, "The project was not found in the projects registry: " + project_name
    );

  writeProjects();
}