#include "objects/Configs/LocalConfig.hh"
#include "files.hh"
#include "helpers.hh"
#include "nlohmann/json.hpp"
#include "objects/Configs/Config.hh"
#include "objects/ZCError.hh"

LocalConfig::LocalConfig(const std::filesystem::path &file) : Config(file)
{
  load();
}

void LocalConfig::load()
{
  nlohmann::json json_conf = parseJsonFile(file_);

  if (!json_conf.contains("name") || !json_conf.contains("target"))
    throw ZCError(ZC_CONFIG_MISSING_PROPERTY, "required properties for project are missing");

  static_compile_ = json_conf.value<bool>("static", false);
  name_ = json_conf.value("name", "");
  author_ = json_conf.value("author", "");
  c_std_ = json_conf.value("c_std", "c17");
  cpp_std_ = json_conf.value("cpp_std", "c++20");
  add_std_ = json_conf.value("add_std", false);
  version_ = Version(json_conf.value("version", "0.0.0"));
  src_folder_ = json_conf.value("srcFolder", "src");
  include_folder_ = json_conf.value("includeFolder", "include");
  target_ = json_conf.value("target", "");

  // Type
  std::string type_str = "";
  if (json_conf.contains("type") && json_conf["type"].is_string())
    type_str = upper(json_conf["type"].get<std::string>());
  if (type_str == "BIN")
    type_ = Type::BIN;
  else if (type_str == "LIB")
    type_ = Type::LIB;
  else
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "The project type is incorrect");
}

void LocalConfig::write() const
{
  nlohmann::json root;
  root["name"] = name_;
  root["author"] = author_;
  root["target"] = target_;
  root["type"] = type_ == Type::BIN ? "bin" : (type_ == Type::LIB) ? "lib" : "";

  if (src_folder_.filename().string() != "src")
    root["srcFolder"] = src_folder_;

  if (include_folder_.filename().string() != "include")
    root["includeFolder"] = include_folder_;

  if (version_)
    root["version"] = version_->string();

  writeJsonFile(root, file_);
}
