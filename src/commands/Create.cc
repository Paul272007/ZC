#include "commands/Command.hh"
#include <filesystem>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include <commands/Create.hh>
#include <interface.hh>
#include <objects/File.hh>
#include <objects/Settings.hh>
#include <objects/ZCError.hh>

using namespace std;
namespace fs = std::filesystem;

Create::Create(
    const vector<string> &files, const bool force, const bool quiet, const vector<string> &input_files,
    const bool edit
)
    : Command(force, quiet), edit_(edit), settings_(Settings::getInstance())
{
  for (const auto &f : files)
    files_.emplace_back(f);
  for (const auto &i : input_files)
    input_files_.emplace_back(i);
}

std::vector<File> Create::getTemplates() const
{
  vector<File> file_list;
  try
  {
    if (fs::exists(templates_path_) && fs::is_directory(templates_path_))
      for (const auto &entry : fs::directory_iterator(templates_path_))
        if (entry.is_regular_file())
          file_list.emplace_back(entry.path().string());
  }
  catch (const fs::filesystem_error &e)
  {
    throw ZCError(ZC_INTERNAL_ERROR, e.what());
  }
  return file_list;
}

int Create::execute()
{
  const vector<File> templates = getTemplates();
  vector<string> files_to_edit;
  for (const auto &f : files_)
  {
    // Check if file already exists
    if (f.exists() && !force_)
      if (!ask("The file " + f.getPath().string() + " already exists. Do you want to replace it ?"))
        continue;
    // If file is a C header
    if (f.getLanguage() == H && !input_files_.empty())
    {
      for (const auto &i_f : input_files_)
      {
        if (!i_f.exists())
          throw ZCError(ZC_NOT_FOUND, "Input file " + i_f.getPath().string() + " not found.");
        if (i_f.getLanguage() != C && i_f.getLanguage() != CPP)
          throw ZCError(
              ZC_UNSUPPORTED_LANGUAGE,
              "Input file " + i_f.getPath().string() + " has an unsupported file type."
          );
      }
      writeCDecls(f);
    }
    // Else use a template
    else
    {
      bool found = false;
      for (const auto &t : templates)
      {
        if (f.getExt() == t.getExt() || (f.getLanguage() != OTHER && f.getLanguage() == t.getLanguage()))
        {
          f.copy(t);
          found = true;
          break;
        }
      }
      if (!found)
        throw ZCError(
            ZC_UNSUPPORTED_LANGUAGE, "No template is available for the file: " + f.getPath().string()
        );
    }
    files_to_edit.push_back(f.getPath());
  }
  if ((settings_.getEditOnCreate() || edit_) && !files_to_edit.empty())
  {
    stringstream cmd;
    cmd << settings_.getEditor();
    for (const auto &f : files_to_edit)
      cmd << " " << f;
    return system(cmd.str().c_str());
  }
  return 0;
}

void Create::writeCDecls(const File &f) const
{
  Declarations all_decls;
  auto merge_decls = [&](const vector<string> &src, vector<string> &dest)
  {
    unordered_set existing(dest.begin(), dest.end());

    for (const auto &item : src)
    {
      if (!existing.contains(item))
      {
        dest.push_back(item);
        // src can have the item twice too
        existing.insert(item);
      }
    }
  };

  for (const auto &file : input_files_)
  {
    unique_ptr<Declarations> d = file.parse();
    merge_decls((*d)["includes"], all_decls["includes"]);
    merge_decls((*d)["macros"], all_decls["macros"]);
    merge_decls((*d)["globals"], all_decls["globals"]);
    merge_decls((*d)["typedefs"], all_decls["typedefs"]);
    merge_decls((*d)["enums"], all_decls["enums"]);
    merge_decls((*d)["functions"], all_decls["functions"]);
    merge_decls((*d)["structs"], all_decls["structs"]);
    merge_decls((*d)["unions"], all_decls["unions"]);
  }
  f.writeDeclarations(all_decls);
}
