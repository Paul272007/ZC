#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <helpers.hh>
#include <nlohmann/json.hpp>

#define CONFIG "config.json"

class Settings
{
public:
  /**
   * @brief Get an instance
   *
   * @return A Settings instance
   */
  static Settings &getInstance();

  /**
   * @brief Load the configuration file
   */
  void load();

  /**
   * @brief Write the current configuration into the configuration file
   */
  // void write();

  std::filesystem::path config_path_ = getZCRootDir() / CONFIG;

  /* Compiling settings */
  bool auto_add_std_ = false;
  std::string c_compiler_ = "clang";
  std::string cpp_compiler_ = "clang++";
  std::string c_std_ = "c17";
  std::string cpp_std_ = "c++20";
  std::vector<std::string> flags_ = {"-Wall", "-Wextra"};

  /* User settings */
  bool clear_before_run_ = false;
  bool auto_keep_ = false;
  bool edit_on_init_ = false;
  bool edit_on_create_ = false;
  std::string editor_ = "nvim";

private:
  /**
   * @brief Default constructor
   */
  Settings();
};