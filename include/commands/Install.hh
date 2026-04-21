#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include <commands/Command.hh>
#include <objects/Registry.hh>
#include <objects/Version.hh>

#define REGISTRY_URL "https://paul272007.github.io/ZC-Registry/index.json"

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

  Install(
      const std::vector<std::pair<std::string, std::string>> &targets, const std::string &path,
      const bool global, const bool force, const bool quiet
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

  std::vector<std::pair<std::string, std::string>> targets_;
  const std::filesystem::path path_;

  Registry registry_;
};
