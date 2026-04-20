#pragma once

#include <filesystem>
#include <string>

#include <commands/Command.hh>
#include <objects/Registry.hh>
#include <vector>

#define REGISTRY_URL "http://localhost:8000/index.json"

class Install : public Command
{
public:
  /**
   * @brief Install a library from server or local zc project
   *
   * @param targets The names of the packages to be installed
   * @param path
   * @param global
   * @param force
   * @param quiet
   */
  Install(
      const std::vector<std::string> &targets, const std::string &path, bool global, bool force, bool quiet
  );

  /**
   * @brief Execute command
   *
   * @return Exit code
   */
  int operator()() override;

private:
  void installFromPath();
  void installFromJson();
  void installFromServer();

  std::vector<std::string> targets_;
  const std::filesystem::path path_;

  Registry registry_;
};
