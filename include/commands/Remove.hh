#pragma once

#include <string>
#include <vector>

#include <commands/Command.hh>
#include <objects/Registry.hh>

class Remove : public Command
{
public:
  /**
   * @brief Uninstall given libraries
   *
   * @param targets The libraries to be installed
   * @param force
   * @param quiet
   * @param global
   */
  Remove(const std::vector<std::string> &targets, bool force, bool quiet, bool global);

  /**
   * @brief Execute command
   *
   * @return Exit code
   */
  int execute() override;

private:
  Registry registry_;
  const std::vector<std::string> targets_;
};
