#include <string>

#include <helpers.hh>
#include <objects/GlobalSettings.hh>
#include <objects/Settings.hh>
#include <objects/ZCError.hh>

using json = nlohmann::json;
using namespace std;

GlobalSettings::GlobalSettings() : Settings(getZCRootDir())
{
  load();
}

GlobalSettings &GlobalSettings::getInstance()
{
  static GlobalSettings instance;
  return instance;
}

void GlobalSettings::write() const
{
}

void GlobalSettings::load()
{
  json json_conf = parseJsonFile(config_path_);

  // Compilers configuration
  c_compiler_ = json_conf.value("c_compiler", "clang");
  cpp_compiler_ = json_conf.value("cpp_compiler", "clang++");

  c_std_ = json_conf.value("c_std", "c17");
  cpp_std_ = json_conf.value("cpp_std", "c++20");

  add_std_ = json_conf.value("auto_add_std", false);

  flags_ = json_conf.value<vector<string>>("flags", vector<string>{"-Wall", "-Wextra"});

  // User settings
  editor_ = json_conf.value("editor", "nvim");
  clear_before_run_ = json_conf.value<bool>("clear_before_run", false);
  auto_keep_ = json_conf.value<bool>("auto_keep", false);
  edit_on_init_ = json_conf.value<bool>("edit_on_init", false);
  edit_on_create_ = json_conf.value<bool>("edit_on_create", false);
}
