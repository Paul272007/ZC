#include <filesystem>
#include <fstream>

#include <objects/Settings.hh>
#include <objects/ZCError.hh>
#include <string>

using json = nlohmann::json;
using namespace std;
namespace fs = std::filesystem;

Settings::Settings()
{
  load();
}

Settings &Settings::getInstance()
{
  static Settings instance;
  return instance;
}

void Settings::load()
{
  json json_conf;
  if (!fs::exists(config_path_))
  {
    throw ZCError(ZC_CONFIG_NOT_FOUND, "The configuration file was not found: " + config_path_.string());
  }
  ifstream input(config_path_);
  if (!input.is_open())
    throw ZCError(
        ZC_CONFIG_READING_ERROR, "The configuration file couldn't be read: " + config_path_.string()
    );
  try
  {
    input >> json_conf;
  }
  catch (const json::parse_error &e)
  {
    throw ZCError(
        ZC_CONFIG_PARSING_ERROR,
        "The configuration file couldn't be parsed: " + config_path_.string() + ": " + e.what()
    );
  }
  // Compilers configuration
  c_compiler_ = json_conf.value("c_compiler", "clang");
  cpp_compiler_ = json_conf.value("cpp_compiler", "clang++");

  c_std_ = json_conf.value("c_std", "c17");
  cpp_std_ = json_conf.value("cpp_std", "c++20");

  auto_add_std_ = json_conf.value("auto_add_std", false);

  flags_ = json_conf.value<vector<string>>("flags", vector<string>{"-Wall", "-Wextra"});

  // User settings
  editor_ = json_conf.value("editor", "nvim");
  clear_before_run_ = json_conf.value<bool>("clear_before_run", false);
  auto_keep_ = json_conf.value<bool>("auto_keep", false);
  edit_on_init_ = json_conf.value<bool>("edit_on_init", false);
  edit_on_create_ = json_conf.value<bool>("edit_on_create", false);
}