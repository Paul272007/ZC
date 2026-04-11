#include <filesystem>
#include <string>
#include <vector>

#include <commands/Init.hh>
#include <helpers.hh>
#include <interface.hh>
#include <objects/ProjectSettings.hh>
#include <objects/Settings.hh>
#include <objects/ZCError.hh>

using namespace std;
namespace fs = std::filesystem;

Init::Init(
    const std::string &author, const std::string &project_template, const std::string &name,
    const std::string &src_folder, const std::string &include_folder, const bool force, const bool quiet,
    const bool edit, const bool git
)
    : Command(force, quiet), edit_(edit), git_(git), settings_(Settings::getInstance()),
      path_(fs::current_path())
{
  if (!force_ && fs::exists(ZC_FILE))
    if (!ask(
            "It seems like a ZC project is already initialized in this directory. Do you want to overwrite "
            "it ?"
        ))
      exit(0);

  // Ask if not precised (default option for the package is the current directory)
  if (name.empty())
    name_ = input("Package name: ", fs::current_path().filename().string());

  if (author.empty())
    author_ = input("Project author: ");

  // std::string type = input("Project type: (lib/bin)");
  // if (type == "lib")
  //   type_ = LIB;
  // else if (type == "bin")
  //   type_ = BIN;

  if (project_template.empty())
    template_ = input("Template to use to initialize project: ");

  // Don't ask to override default settings for these
  if (!src_folder.empty())
    src_folder_ = src_folder;

  if (!include_folder.empty())
    include_folder_ = include_folder;
}

vector<fs::path> Init::getProjectTemplates() const
{
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

int Init::execute()
{
  if (!template_.empty())
  {
    const vector<fs::path> templates_list = getProjectTemplates();
    bool found = false;

    for (const auto &dir : templates_list)
    {
      if (split(dir, '.').back() == template_)
      {
        for (const auto &entry : fs::recursive_directory_iterator(dir))
        {
          // For each entry in the template, check if it already exists here
          const fs::path &src_path = entry.path();

          fs::path rel_path = fs::relative(src_path, dir);
          fs::path dest_path = path_ / rel_path;

          // Only copy gitignore if git is enabled on project
          if (src_path.filename() == ".gitignore" && !git_)
            continue;

          if (fs::exists(dest_path) && !force_)
            if (!ask("The entry " + dest_path.string() + " already exists. Do you want to overwrite it ?"))
              continue;

          // Copy stuff
          if (fs::is_symlink(src_path))
          {
            fs::path target = fs::read_symlink(src_path);
            // Directory symlinks
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

        found = true;
        break;
      }
    }

    if (!found)
      throw ZCError(ZC_UNSUPPORTED_LANGUAGE, "The following template was not found: " + template_);
  }

  if (git_)
  {
    info("Initializing git repo...");
    if (system("git init") != 0)
      throw ZCError(ZC_GIT_ERROR, "Git init failed");
  }

  // Create configuration file with empty shared lib, static lib, executable, type and dependencies
  ProjectSettings settings(name_, author_, "", "", "", src_folder_, include_folder_, UNDEF, {});
  settings.write();

  if (settings_.getEditOnInit() || edit_)
    return system(string(settings_.getEditor() + " " + path_.string()).c_str());

  return 0;
}
