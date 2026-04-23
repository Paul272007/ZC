#include "objects/Configs/GlobalConfig.hh"
#include "files.hh"
#include "nlohmann/json.hpp"
#include "objects/Configs/Config.hh"

GlobalConfig::GlobalConfig(const std::filesystem::path &file) : Config(file)
{
  load();
}

void GlobalConfig::write() const
{
}

void GlobalConfig::load()
{
  nlohmann::json json_conf = parseJsonFile(file_);

  c_compiler_ = json_conf.value("c_compiler", "clang");
  cpp_compiler_ = json_conf.value("cpp_compiler", "clang++");

  c_std_ = json_conf.value("c_std", "c17");
  cpp_std_ = json_conf.value("cpp_std", "c++20");
  add_std_ = json_conf.value("auto_add_std", false);

  editor_ = json_conf.value("editor", "nvim");

  flags_ = json_conf.value<std::vector<std::string>>("flags", std::vector<std::string>{"-Wall", "-Wextra"});

  move_binary_to_current_path_ = json_conf.value<bool>("move_binary_to_current_path", false);
  clear_before_run_ = json_conf.value<bool>("clear_before_run", false);
  auto_keep_ = json_conf.value<bool>("auto_keep", false);
  edit_on_init_ = json_conf.value<bool>("edit_on_init", false);
  edit_on_create_ = json_conf.value<bool>("edit_on_create", false);
}
