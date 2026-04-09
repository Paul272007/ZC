#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <commands/Command.hh>
#include <helpers.hh>
#include <objects/File.hh>
#include <objects/Settings.hh>

#define TEMPLATES "templates"

class Create : public Command
{
public:
  /**
   * @brief Create an instance of the Init command
   *
   * @param files The files to be initialized
   * @param force Force creation even if the files already exist
   * @param input_files Files to be used as basis to create the new ones
   * @param edit Edit file after creating it
   */
  Create(
      const std::vector<std::string> &files, bool force, bool quiet,
      const std::vector<std::string> &input_files, bool edit
  );

  /**
   * @brief Execute function
   *
   * @return return code depending on whether the function ended
   * successfully
   */
  int execute() override;

private:
  /**
   * @brief Return all the templates that exist for the specific file type
   */
  std::vector<File> getTemplates() const;

  /**
   * @brief Write the C declarations into the header file
   *
   * @param f The file to be written
   * @return Whether the file was written successfully
   */
  void writeCDecls(const File &f) const;

  bool edit_;
  Settings &settings_;
  std::vector<File> files_;
  std::vector<File> input_files_;
  std::filesystem::path templates_path_ = getZCRootDir() / TEMPLATES;
};