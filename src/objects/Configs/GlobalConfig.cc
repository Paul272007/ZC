#include "objects/Configs/GlobalConfig.hh"
#include "files.hh"
#include "nlohmann/json.hpp"
#include "objects/Configs/Config.hh"
#include <sys/stat.h>

using json = nlohmann::json;

GlobalConfig::GlobalConfig(const std::filesystem::path &file) : Config(file)
{
  load();
}

void GlobalConfig::write() const
{
  json root;
  root["c_compiler"] = c_compiler_;
  root["cpp_compiler"] = cpp_compiler_;
  root["c_std"] = c_std_;
  root["cpp_std"] = cpp_std_;
  root["auto_add_std"] = add_std_;
  root["editor"] = editor_;
  root["flags"] = flags_;
  root["move_binary_to_current_path"] = move_binary_to_current_path_;
  root["clear_before_run"] = clear_before_run_;
  root["auto_keep"] = auto_keep_;
  root["edit_on_init"] = edit_on_init_;
  root["edit_on_create"] = edit_on_create_;

  if (!token_.empty())
    root["token"] = token_;
  if (!default_author_.empty())
    root["username"] = default_author_;

  writeJsonFile(root, file_);
  chmod(file_.c_str(), S_IRUSR | S_IWUSR);
}

void GlobalConfig::load()
{
  json json_conf = parseJsonFile(file_);

  c_compiler_ = json_conf.value("c_compiler", "clang");
  cpp_compiler_ = json_conf.value("cpp_compiler", "clang++");

  c_std_ = json_conf.value("c_std", "c17");
  cpp_std_ = json_conf.value("cpp_std", "c++20");
  add_std_ = json_conf.value("auto_add_std", false);

  editor_ = json_conf.value("editor", "nvim");
  token_ = json_conf.value("token", "");
  default_author_ = json_conf.value("username", "");

  flags_ = json_conf.value<std::vector<std::string>>("flags", std::vector<std::string>{"-Wall", "-Wextra"});

  move_binary_to_current_path_ = json_conf.value<bool>("move_binary_to_current_path", false);
  clear_before_run_ = json_conf.value<bool>("clear_before_run", false);
  auto_keep_ = json_conf.value<bool>("auto_keep", false);
  edit_on_init_ = json_conf.value<bool>("edit_on_init", false);
  edit_on_create_ = json_conf.value<bool>("edit_on_create", false);
}
