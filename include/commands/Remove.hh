#pragma once

#include <string>
#include <vector>

#include <commands/Command.hh>
#include <objects/ProjectSettings.hh>
#include <objects/Registry.hh>

class Remove : public Command
{
public:
  /**
   * @brief Uninstall given libraries
   *
   * @param targets The libraries to be installed
   */
  Remove(const std::vector<std::string> &targets, const bool force, const bool quiet, const bool global);

  /**
   * @brief Execute command
   *
   * @return Exit code
   */
  int execute() override;

private:
  Registry *registry_ = nullptr;
  ProjectSettings *p_settings_ = nullptr;
  const bool global_;
  const std::vector<std::string> targets_;
};
