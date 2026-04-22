#include <filesystem>
#include <fstream>
#include <sstream>

#include "commands/Create.hh"
#include "files.hh"
#include "objects/Controller.hh"
#include "objects/GlobalController.hh"
#include "objects/ZCError.hh"

using namespace std;
namespace fs = std::filesystem;

Create::Create(
    bool force, bool quiet, bool edit, const std::vector<std::string> &files,
    const std::vector<std::string> &input_files
)
    : Command(force, quiet), edit_(edit), g_(logger_, force)
{
  for (const auto &f : files)
    files_.emplace_back(f);
  for (const auto &i : input_files)
    input_files_.emplace_back(i);
}

int Create::operator()()
{
  const vector<fs::path> templates = g_.getTemplates();
  vector<string> files_to_edit;
  for (const auto &f : files_)
  {
    // Check if file already exists
    if (fs::exists(f) && !force_)
      if (!ask("The file '" + f.string() + "' already exists. Do you want to replace it ?"))
        continue;
    // If file is a C header
    if (f.extension() == ".h" && !input_files_.empty())
    {
      for (const auto &i_f : input_files_)
      {
        if (!fs::exists(i_f))
          throw ZCError(ZC_NOT_FOUND, "Input file " + i_f.string() + " not found.");
        if (i_f.extension() != ".c" && !isCpp(i_f))
          throw ZCError(
              ZC_UNSUPPORTED_LANGUAGE, "Input file " + i_f.string() + " has an unsupported file type."
          );
      }
      writeCDecls(f);
      logger_(LogLevel::SUCCESS, "fs::path written: " + f.string());
    }
    else // Else use a template
    {
      bool found = false;
      for (const auto &t : templates)
      {
        if (f.extension() == t.extension() || isCpp(f) && isCpp(t))
        {
          ifstream t_file(t);
          if (!t_file.is_open())
            throw ZCError(ZC_READING_ERROR, "Could not read " + t.string());

          ofstream file(f);
          if (!file.is_open())
            throw ZCError(ZC_WRITING_ERROR, "Could not write into " + f.string());

          file << t_file.rdbuf();

          logger_(LogLevel::SUCCESS, "fs::path written: " + f.string());
          found = true;
          break;
        }
      }
      if (!found)
        throw ZCError(ZC_UNSUPPORTED_LANGUAGE, "No template is available for the file: " + f.string());
    }
    files_to_edit.push_back(f);
  }
  if ((g_.gc_->edit_on_create_ || edit_) && !files_to_edit.empty())
  {
    stringstream cmd;
    cmd << g_.gc_->editor_;
    for (const auto &f : files_to_edit)
      cmd << " " << f;
    return system(cmd.str().c_str());
  }
  return 0;
}

void Create::writeCDecls(const fs::path &f) const
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
    unique_ptr<Declarations> d = parse(file);
    merge_decls((*d)["includes"], all_decls["includes"]);
    merge_decls((*d)["macros"], all_decls["macros"]);
    merge_decls((*d)["globals"], all_decls["globals"]);
    merge_decls((*d)["typedefs"], all_decls["typedefs"]);
    merge_decls((*d)["enums"], all_decls["enums"]);
    merge_decls((*d)["functions"], all_decls["functions"]);
    merge_decls((*d)["structs"], all_decls["structs"]);
    merge_decls((*d)["unions"], all_decls["unions"]);
  }
  writeDeclarations(all_decls, f);
}
