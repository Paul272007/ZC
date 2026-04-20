#include <filesystem>

#include <fstream>
#include <helpers.hh>
#include <objects/ProjectSettings.hh>
#include <objects/ZCError.hh>
#include <string>

using json = nlohmann::json;
using namespace std;
namespace fs = std::filesystem;

ProjectSettings::ProjectSettings(const std::filesystem::path &project_root)
    : project_root_(project_root), config_file_(project_root_ / ZC_FILE)
{
  load();
}

ProjectSettings::ProjectSettings(
    const std::string &name, const std::string &author, const std::string &targetName,
    const std::string &version, const std::string &src, const std::string &include, const ProjectType &type
)
    : config_file_(fs::current_path() / ZC_FILE), type_(type), name_(name), author_(author),
      target_name_(targetName), version_(version), src_folder_(src), include_folder_(include)
{
  // If the project is being created (and does not exist yet), the root is the current path and zc.json
  // doesn't exist yet
}

void ProjectSettings::load()
{
  json json_conf;
  if (!fs::exists(config_file_))
  {
    throw ZCError(
        ZC_CONFIG_NOT_FOUND, "The project configuration file was not found: " + config_file_.string()
    );
  }
  ifstream input(config_file_);
  if (!input.is_open())
  {
    throw ZCError(
        ZC_CONFIG_READING_ERROR, "The project configuration file couldn't be read: " + config_file_.string()
    );
  }
  try
  {
    input >> json_conf;
  }
  catch (const json::parse_error &e)
  {
    throw ZCError(
        ZC_CONFIG_PARSING_ERROR,
        "The configuration file couldn't be parsed: " + config_file_.string() + ": " + e.what()
    );
  }
  if (!json_conf.contains("name"))
  {
    throw ZCError(ZC_CONFIG_MISSING_PROPERTY, "required properties for project are missing");
  }
  name_ = json_conf.value("name", "");
  author_ = json_conf.value("author", "");
  version_ = Version(json_conf.value("version", "0.0.0"));
  src_folder_ = project_root_ / json_conf.value("srcFolder", "src");
  include_folder_ = project_root_ / json_conf.value("includeFolder", "include");
  target_name_ = json_conf.value("target", "");

  // Type and output
  string type_str = "";
  if (json_conf.contains("type") && json_conf["type"].is_string())
    type_str = upper(json_conf["type"].get<string>());
  if (type_str == "BIN")
    type_ = BIN;
  else if (type_str == "LIB")
    type_ = LIB;
  else
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "The project type is incorrect");
}

void ProjectSettings::write() const
{
  json root;
  root["name"] = name_;
  root["author"] = author_;
  root["target"] = target_name_;
  root["type"] = type_ == BIN ? "bin" : (type_ == LIB) ? "lib" : "";

  if (src_folder_ != "src")
    root["srcFolder"] = src_folder_;

  if (include_folder_ != "include")
    root["includeFolder"] = include_folder_;

  if (version_)
    root["version"] = version_->string();

  ofstream output(config_file_);
  if (!output.is_open())
  {
    throw ZCError(
        ZC_CONFIG_WRITING_ERROR, "The project configuration couldn't be written: " + config_file_.string()
    );
  }
  output << root.dump(2);
  output.close();
}
