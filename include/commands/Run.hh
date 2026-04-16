#pragma once

#include <objects/File.hh>
#include <objects/Registry.hh>
#include <objects/Settings.hh>
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
      bool preprocess, bool compile, bool assemble, bool force, bool quiet, bool add_std);

  /**
   * @brief Execute command
   *
   * @return Exit code
   */
  int operator()() override;

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
   * @return Whether to compile as C++
   */
  bool isCppAndCheckExtensions() const;

  /**
   * @brief build the compiling command
   */
  void buildCommand();

  /**
   * @brief Get library inclusions from file
   */
  std::vector<std::string> getInclusions() const;

  const Settings &settings_;
  const Registry registry_;
  const bool add_std_;
  const bool keep_ = false;
  bool plus_ = false;
  std::string output_name_;
  std::string build_cmd_;
  Mode mode_ = FULL;
  std::vector<File> files_;
  std::vector<std::string> args_;
};
