#pragma once

#include "objects/Registry.hh"
#include "objects/Settings.hh"
#include <filesystem>
#include <objects/File.hh>
#include <string>
#include <vector>

#include <commands/Command.hh>

enum Mode
{
  FULL,
  PREPROCESS,
  COMPILE,
  ASSEMBLE
};

class Run : public Command
{
public:
  /**
   * @brief Compile given files and execute program if the output is executable
   *
   * @param files The files to be compiled (and execute)
   * @param args The arguments to be passed to the program once executed
   * @param keep Whether to keep the executable once executed or not
   * @param plus Whether to force compilation as C++
   * @param preprocess Preprocess only
   * @param compile Preprocess and compile only
   * @param assemble Preprocess, compile and assemble only
   * @param quiet Enable quiet mode for output
   */
  Run(const std::vector<std::string> &files, const std::vector<std::string> &args, bool keep, bool plus,
      bool preprocess, bool compile, bool assemble, bool force, bool quiet);

  /**
   * @brief Execute command
   *
   * @return Exit code
   */
  int execute() override;

private:
  /**
   * @brief Get compiling mode and check validity of the command, and throw an
   * error if the command is not valid
   *
   * @param preprocess Whether the preprocess flag is given or not
   * @param compile Whether the compile flag is given or not
   * @param assemble Whether the "assemble" flag is given or not
   * @return The compiling mode found
   */
  static Mode getMode(bool preprocess, bool compile, bool assemble);

  /**
   * @brief Check if we must compile as C++ and check file extensions
   *
   * @param badFile Fill this variable with the file that is faulty
   * @return Whether to compile as C++
   */
  bool isCppAndCheckExtensions(std::filesystem::path &badFile) const;

  /**
   * @brief build the compiling command
   *
   * @param output_name The name of the output of the command
   */
  std::string buildCommand(const std::string &output_name) const;

  /**
   * @brief Check that all files exist
   *
   * @param badFile Fill this variable with the file that is faulty
   * @return Whether all files exist
   */
  bool filesExist(std::filesystem::path &badFile) const;

  /**
   * @brief Get library inclusions from file
   */
  std::vector<std::string> getInclusions() const;

  bool keep_ = false;

  bool plus_ = false;

  Mode mode_ = FULL;

  Settings &settings_;

  Registry &registry_;

  std::vector<File> files_;

  std::vector<std::string> args_;
};
