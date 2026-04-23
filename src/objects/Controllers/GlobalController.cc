#include <filesystem>
#include <string>

#include "helpers.hh"
#include "interface.hh"
#include "objects/Configs/GlobalConfig.hh"
#include "objects/Controllers/Controller.hh"
#include "objects/Controllers/GlobalController.hh"
#include "objects/Registries/GlobalRegistry.hh"
#include "objects/ZCError.hh"

namespace fs = std::filesystem;

GlobalController::GlobalController(Logger log, bool force) : Controller(log, force, getZCRootDir())
{
  std::filesystem::path bin_dir_ = root_dir_ / BIN_DIR;
  std::filesystem::path lib_dir_ = root_dir_ / LIB_DIR;
  std::filesystem::path include_dir_ = root_dir_ / INCLUDE_DIR;
  gc_ = new GlobalConfig(root_dir_ / CONFIG);
  c_ = gc_;
  r_ = new GlobalRegistry(root_dir_ / REGISTRY);
}

void GlobalController::initializeWithTemplate(
    const std::filesystem::path &root, const std::string &template_to_use
) const
{
  if (!fs::exists(templates_dir_))
    throw ZCError(ZC_NOT_FOUND, "The following template was not found: " + template_to_use);

  for (const auto &entry : fs::recursive_directory_iterator(templates_dir_))
  {
    // For each entry in the template, check if it already exists here
    const fs::path &src_path = entry.path();

    fs::path rel_path = fs::relative(src_path, templates_dir_);
    fs::path dest_path = root / rel_path;

    if (fs::exists(dest_path) && !force_)
      if (!ask("The entry " + dest_path.string() + " already exists. Do you want to overwrite it ?"))
        continue;

    // Copy stuff
    if (fs::is_symlink(src_path))
    {
      fs::path target = fs::read_symlink(src_path);

      if (fs::is_directory(src_path))
        fs::create_directory_symlink(target, dest_path);
      else
        fs::create_symlink(target, dest_path);
    }
    else if (fs::is_directory(src_path))
    {
      fs::create_directories(dest_path);
    }
    else if (fs::is_regular_file(src_path))
    {
      fs::create_directories(dest_path.parent_path());
      fs::copy_file(src_path, dest_path, fs::copy_options::overwrite_existing);
    }
  }
}

std::vector<fs::path> GlobalController::getProjectTemplates() const
{
  std::vector<fs::path> templates_list;
  try
  {
    if (fs::exists(p_templates_dir_) && fs::is_directory(p_templates_dir_))
      for (const auto &entry : fs::directory_iterator(p_templates_dir_))
        if (entry.is_directory())
          templates_list.push_back(entry.path());
  }
  catch (const fs::filesystem_error &e)
  {
    throw ZCError(ZC_INTERNAL_ERROR, e.what());
  }
  return templates_list;
}

std::vector<fs::path> GlobalController::getTemplates() const
{
  std::vector<fs::path> file_list;
  try
  {
    if (fs::exists(templates_dir_) && fs::is_directory(templates_dir_))
      for (const auto &entry : fs::directory_iterator(templates_dir_))
        if (entry.is_regular_file())
          file_list.emplace_back(entry.path());
  }
  catch (const fs::filesystem_error &e)
  {
    throw ZCError(ZC_INTERNAL_ERROR, e.what());
  }
  return file_list;
}

Table GlobalController::projectTemplatesTable() const
{
  std::vector<std::vector<std::string>> str_p_t = {{"Name"}};
  const auto templates = getProjectTemplates();
  for (const auto &t_path : templates)
    str_p_t.push_back({t_path.filename().string()});

  return {static_cast<int>(str_p_t.size()), 1, false, true, str_p_t};
}

Table GlobalController::templatesTable() const
{
  std::vector<std::vector<std::string>> str_t = {{"Template"}};
  const auto templates = getTemplates();
  for (const auto &t_path : templates)
    str_t.push_back({t_path.filename().string()});

  return {static_cast<int>(str_t.size()), 1, false, true, str_t};
}
