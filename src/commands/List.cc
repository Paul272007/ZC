#include <filesystem>
#include <vector>

#include <commands/Command.hh>
#include <commands/List.hh>
#include <helpers.hh>
#include <objects/Table.hh>
#include <objects/ZCError.hh>

using namespace std;
namespace fs = std::filesystem;

List::List(
    const bool force, const bool quiet, const bool global, const bool templates, const bool p_templates
)
    : Command(force, quiet), registry_(global ? Registry() : Registry(getProjectRoot())),
      templates_(templates), p_templates_(p_templates)
{
}

vector<fs::path> List::getProjectTemplates() const
{
  std::filesystem::path project_templates_path_ = getZCRootDir() / PROJECT_TEMPLATES;
  vector<fs::path> templates_list;
  try
  {
    if (fs::exists(project_templates_path_) && fs::is_directory(project_templates_path_))
      for (const auto &entry : fs::directory_iterator(project_templates_path_))
        if (entry.is_directory())
          templates_list.push_back(entry.path());
  }
  catch (const fs::filesystem_error &e)
  {
    throw ZCError(ZC_INTERNAL_ERROR, e.what());
  }
  return templates_list;
}

Table List::projectTemplatesTable() const
{
  vector<vector<string>> str_p_t = {{"Name"}};
  const auto templates = getProjectTemplates();
  for (const auto &t_path : templates)
    str_p_t.push_back({t_path.filename().string()});

  return {static_cast<int>(str_p_t.size()), 1, false, true, str_p_t};
}

vector<File> List::getTemplates() const
{
  std::filesystem::path templates_path_ = getZCRootDir() / TEMPLATES;
  vector<File> file_list;
  try
  {
    if (fs::exists(templates_path_) && fs::is_directory(templates_path_))
      for (const auto &entry : fs::directory_iterator(templates_path_))
        if (entry.is_regular_file())
          file_list.emplace_back(entry.path());
  }
  catch (const fs::filesystem_error &e)
  {
    throw ZCError(ZC_INTERNAL_ERROR, e.what());
  }
  return file_list;
}

Table List::templatesTable() const
{
  vector<vector<string>> str_t = {{"Language"}};
  const auto templates = getTemplates();
  for (const auto &t_path : templates)
    str_t.push_back({to_string(t_path.getLanguage())});

  return {static_cast<int>(str_t.size()), 1, false, true, str_t};
}

int List::operator()()
{
  Table t(
      templates_ ? templatesTable() : (p_templates_ ? projectTemplatesTable() : registry_.packagesTable())
  );
  if (t.getSize() < 2)
    log_info("Nothing to show.");
  else
    t.draw();

  return 0;
}
