#pragma once

#include <filesystem>
#include <string>

#include <commands/Command.hh>
#include <objects/Registry.hh>
#include <vector>

class Install : public Command
{
public:
  /**
   * @brief Install a library from server or local zc project
   *
   * @param targets The names of the packages to be installed
   */
  Install(
      const std::vector<std::string> &targets, const std::string &path, const bool global, const bool force,
      const bool quiet
  );

  /**
   * @brief Execute command
   *
   * @return Exit code
   */
  int execute() override;

private:
  void installFromPath() const;

  const std::vector<std::string> targets_;
  const std::filesystem::path path_;
  const bool global_;

  Registry &registry_;
};
