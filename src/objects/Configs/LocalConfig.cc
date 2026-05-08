#include "objects/Configs/LocalConfig.hh"
#include "files.hh"
#include "helpers.hh"
#include "nlohmann/json.hpp"
#include "objects/Configs/Config.hh"
#include "objects/ZCError.hh"
#include <vector>

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

  if (json_conf.contains("c_std"))
    c_std_ = json_conf["c_std"];
  else
    c_std_ = std::nullopt;

  if (json_conf.contains("cpp_std"))
    cpp_std_ = json_conf["cpp_std"];
  else
    cpp_std_ = std::nullopt;

  if (json_conf.contains("add_std"))
    add_std_ = json_conf["add_std"];
  else
    add_std_ = std::nullopt;

  version_ = Version(json_conf.value("version", "0.0.0"));
  src_folder_ = json_conf.value("srcFolder", "src");
  include_folder_ = json_conf.value("includeFolder", "include");
  target_ = json_conf.value("target", "");
  languages_ = json_conf.value<std::vector<std::string>>("languages", {"C", "CXX"});

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

  if (c_std_)
    root["c_std"] = *c_std_;
  if (cpp_std_)
    root["cpp_std"] = *cpp_std_;
  if (add_std_)
    root["add_std"] = *add_std_;

  if (src_folder_.filename().string() != "src")
    root["srcFolder"] = src_folder_;

  if (include_folder_.filename().string() != "include")
    root["includeFolder"] = include_folder_;

  if (version_)
    root["version"] = version_->string();

  writeJsonFile(root, file_);
}
