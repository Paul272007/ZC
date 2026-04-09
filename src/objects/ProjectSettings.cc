#include <filesystem>

#include "objects/ZCError.hh"
#include <fstream>
#include <objects/ProjectSettings.hh>

using json = nlohmann::json;
using namespace std;
namespace fs = std::filesystem;

ProjectSettings::ProjectSettings()
{
  load();
}

ProjectSettings::ProjectSettings(
    const std::string &name, const std::string &author, const std::string &sharedLib,
    const std::string &staticLib, const std::string &version, const std::string &src,
    const std::string &include
)
    : name_(name), author_(author), shared_lib_name_(sharedLib), static_lib_name_(staticLib),
      version_(version), src_folder_(src), include_folder_(include)
{
}

void ProjectSettings::load()
{
  json json_conf;
  if (!fs::exists(config_file_))
  {
    throw new ZCError(
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
  name_ = json_conf.value("name", "");
  author_ = json_conf.value("author", "");
  shared_lib_name_ = json_conf.value("shared", "");
  static_lib_name_ = json_conf.value("static", "");
  executable_name_ = json_conf.value("executable", "");
  if ((!shared_lib_name_.empty() || !static_lib_name_.empty()) && !executable_name_.empty())
  {
    throw ZCError(
        ZC_CONFIG_CONTENT_ERROR,
        "The project cannot compile into an executable and libraries at the same time"
    );
  }
  version_ = json_conf.value("version", "0.0.0");
  src_folder_ = json_conf.value("srcFolder", "src");
  include_folder_ = json_conf.value("includeFolder", "include");
}

void ProjectSettings::write() const
{
  json root;
  root["name"] = name_;
  root["author"] = author_;
  root["shared"] = shared_lib_name_;
  root["static"] = static_lib_name_;
  root["executable"] = executable_name_;
  root["version"] = version_->to_string();
  root["srcFolder"] = src_folder_;
  root["includeFolder"] = include_folder_;
  ofstream output(config_file_);
  if (!output.is_open())
  {
    throw ZCError(
        ZC_CONFIG_WRITING_ERROR, "The project configuration couldn't be written: " + config_file_.string()
    );
  }
  output << root.dump(4);
  output.close();
}

const std::string &ProjectSettings::getName() const
{
  return name_;
}
const std::string &ProjectSettings::getAuthor() const
{
  return author_;
}
const std::string &ProjectSettings::getSharedLibName() const
{
  return shared_lib_name_;
}
const std::string &ProjectSettings::getStaticLibName() const
{
  return static_lib_name_;
}
const std::string &ProjectSettings::getExecutableName() const
{
  return executable_name_;
}
Version ProjectSettings::getVersion() const
{
  return version_.value();
}
const std::filesystem::path &ProjectSettings::getSrcFolder() const
{
  return src_folder_;
}
const std::filesystem::path &ProjectSettings::getIncludeFolder() const
{
  return include_folder_;
}