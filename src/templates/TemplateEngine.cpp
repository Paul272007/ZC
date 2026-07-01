#include "TemplateEngine.h"

#include <filesystem>
#include <string>

#include "config/Language.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "ui/Interface.h"

ZC_DEV_CONFIG

namespace zc
{

TemplateEngine &TemplateEngine::get()
{
  static TemplateEngine instance(zc_root());
  return instance;
}

const std::vector<std::string> &TemplateEngine::templates() const
{
  static vector<string> list = [&]
  {
    vector<string> templates_list;
    try
    {
      if (fs::exists(templates_dir_) && fs::is_directory(templates_dir_))
        for (const auto &entry : fs::directory_iterator(templates_dir_))
          if (entry.is_regular_file())
            templates_list.emplace_back(entry.path().filename());
    }
    catch (const fs::filesystem_error &e)
    {
      throw ZCException(ZCE_INTERNAL_ERROR, e.what());
    }
    return templates_list;
  }();
  return list;
}

const std::vector<std::string> &TemplateEngine::p_templates() const
{
  static vector<string> list = [&]
  {
    vector<string> templates_list;
    try
    {
      if (fs::exists(p_templates_dir_) && fs::is_directory(p_templates_dir_))
        for (const auto &entry : fs::directory_iterator(p_templates_dir_))
          if (entry.is_directory())
            templates_list.emplace_back(entry.path().filename());
    }
    catch (const fs::filesystem_error &e)
    {
      throw ZCException(ZCE_INTERNAL_ERROR, e.what());
    }
    return templates_list;
  }();
  return list;
}

Table TemplateEngine::templates_table() const
{
  vector<vector<string>> str_t = { { "Template" } };
  for (const auto &t_path : templates())
    str_t.push_back({ t_path });

  return { false, true, str_t };
}

Table TemplateEngine::p_templates_table() const
{
  vector<vector<string>> str_t = { { "Project Template" } };
  for (const auto &t_path : p_templates())
    str_t.push_back({ t_path });

  return { false, true, str_t };
}

bool TemplateEngine::init_with_template(const std::filesystem::path &file, Language l) const
{
  const vector<string> &ts = templates();
  vector<string>        found_templates_for_language;

  string template_to_use;

  for (CAA t : ts)
    if (is_of_language(l, templates_dir_ / t))
      found_templates_for_language.push_back(t);

  if (found_templates_for_language.empty())
  {
    ui().error("Skipped file " + file.string() + ": no template available");
    return false;
  }
  if (found_templates_for_language.size() > 1)
    template_to_use = found_templates_for_language[ui().radios(
      "Which template do you want to use for file " + file.string(), found_templates_for_language
    )];
  else
    template_to_use = found_templates_for_language.at(0);

  return fs::copy_file(templates_dir_ / template_to_use, file, fs::copy_options::overwrite_existing);
}

void TemplateEngine::init_with_p_template(
  const std::filesystem::path &root, const std::string &p_template, const bool force
) const
{
  if (p_template == "none")
    return;

  const fs::path t_path = p_templates_dir_ / p_template;
  if (!fs::exists(t_path))
    throw ZCException(ZCE_NOT_FOUND, "The template was not found: " + p_template);

  for (const auto &entry : fs::recursive_directory_iterator(t_path))
  {
    const fs::path &src_path = entry.path();
    const fs::path  rel_path = fs::relative(src_path, t_path);

    if (rel_path.empty() || rel_path == ".")
      continue;

    fs::path dest_path = root / rel_path;
    if (fs::exists(dest_path) && !force)
      if (!ui().ask(
            "The entry '" + pretty_path(dest_path) + "' already exists. Do you want to overwrite it ?"
          ))
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

TemplateEngine::TemplateEngine(const std::filesystem::path &root)
  : templates_dir_(root / TEMPLATES_DIR), p_templates_dir_(root / P_TEMPLATES_DIR)
{
}

} // namespace zc
