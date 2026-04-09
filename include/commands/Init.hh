#pragma once

#include <filesystem>
#include <string>

#include <commands/Command.hh>
#include <helpers.hh>
#include <objects/ProjectsRegistry.hh>
#include <objects/Settings.hh>
#include <vector>

#define PROJECT_TEMPLATES "project_templates"

class Init : public Command
{
public:
  /**
   * @brief Create a new project in a new folder using ZC
   *
   * @param author The author
   * @param project_template The template used to initialize the project
   * @param name The name of the new project
   * @param src_folder The name of the src folder
   * @param include_folder The name of the include folder
   * @param force Whether to force creating project even if it already exists in the registry
   * @param quiet Whether to show messages/warnings or not
   * @param edit Whether to open the project in the editor once initialized
   * @param git Whether to initialize empty git repository
   */
  Init(
      const std::string &author, const std::string &project_template, const std::string &name,
      const std::string &src_folder, const std::string &include_folder, const bool force, const bool quiet,
      const bool edit, const bool git
  );

  /**
   * @brief Execute command
   *
   * @return Exit code
   */
  int execute() override;

private:
  /**
   * @brief Fetch all templates from the project templates folder
   */
  [[nodiscard]] std::vector<std::filesystem::path> getProjectTemplates() const;

  std::vector<std::string> project_templates_;
  std::filesystem::path path_;
  std::string author_;
  std::string template_;
  std::string name_;
  std::string src_folder_ = "src/";
  std::string include_folder_ = "include/";
  const bool edit_;
  const bool git_;
  Settings &settings_;
  ProjectsRegistry &p_registry_;
  std::filesystem::path project_templates_path_ = getZCRootDir() / PROJECT_TEMPLATES;
};