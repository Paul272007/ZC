#include <filesystem>

#include "helpers.hh"
#include "objects/ZCError.hh"
#include <fstream>
#include <objects/ProjectSettings.hh>
#include <string>

using json = nlohmann::json;
using namespace std;
namespace fs = std::filesystem;

ProjectSettings::ProjectSettings() : project_root_(getProjectRoot()), config_file_(project_root_ / ZC_FILE)
{
  // If the project already exists, search for its root to find the zc file
  load();
}

ProjectSettings::ProjectSettings(const std::filesystem::path &project_root)
    : project_root_(project_root), config_file_(project_root_ / ZC_FILE)
{
  // If the project already exists, search for its root to find the zc file
  load();
}

ProjectSettings &ProjectSettings::getInstance()
{
  static ProjectSettings instance;
  return instance;
}

ProjectSettings::ProjectSettings(
    const std::string &name, const std::string &author, const std::string &sharedLib,
    const std::string &staticLib, const std::string &version, const std::string &src,
    const std::string &include, const ProjectType &type, const dependencies &deps
)
    : name_(name), author_(author), shared_lib_name_(sharedLib), static_lib_name_(staticLib),
      version_(version), src_folder_(src), include_folder_(include), type_(type), deps_(deps),
      config_file_(fs::current_path() / ZC_FILE)
{
  // If the project is being created (and does not exist yet), the root is the current path
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
  name_ = json_conf.value("name", "");
  author_ = json_conf.value("author", "");
  version_ = json_conf.value("version", "0.0.0");
  src_folder_ = project_root_ / json_conf.value("srcFolder", "src");
  include_folder_ = project_root_ / json_conf.value("includeFolder", "include");

  // Type and output
  string type_str = "";
  if (json_conf.contains("type") && json_conf["type"].is_string())
    type_str = upper(json_conf["type"].get<string>());
  if (type_str == "BIN")
    type_ = BIN;
  else if (type_str == "LIB")
    type_ = LIB;
  else
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "The project type is uncorrect");

  switch (type_)
  {
  case BIN:
    executable_name_ = json_conf.value("executable", "");
    if (executable_name_.empty())
      throw ZCError(ZC_CONFIG_CONTENT_ERROR, "Executable is required when type is set to 'bin'");
    break;
  case LIB:
    shared_lib_name_ = json_conf.value("shared", "");
    static_lib_name_ = json_conf.value("static", "");
    if (static_lib_name_.empty() && shared_lib_name_.empty())
      throw ZCError(
          ZC_CONFIG_CONTENT_ERROR,
          "Library must compile into at least one type of library when type is set to 'lib'"
      );
    break;
  case UNDEF:
    break;
  }

  // Dependencies
  if (json_conf.contains("dependencies") && json_conf["dependencies"].is_object())
    for (auto &[key, value] : json_conf["dependencies"].items())
      deps_.push_back(dependency(key, value));
}

void ProjectSettings::write() const
{
  json root;
  root["name"] = name_;
  root["type"] = type_ == BIN ? "bin" : (type_ == LIB) ? "lib" : "";
  root["author"] = author_;
  root["srcFolder"] = src_folder_;
  root["includeFolder"] = include_folder_;
  if (type_ == LIB)
  {
    root["shared"] = shared_lib_name_;
    root["static"] = static_lib_name_;
  }
  else if (type_ == BIN)
  {
    root["executable"] = executable_name_;
  }
  if (version_)
    root["version"] = version_->to_string();
  root["dependencies"] = json::object();
  for (const auto &[name, version] : deps_)
  {
    root["dependencies"][name] = version.to_string();
  }
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
const std::filesystem::path ProjectSettings::getConfigFile() const
{
  return config_file_;
}
const dependencies ProjectSettings::getDeps() const
{
  return deps_;
}
const ProjectType ProjectSettings::getType() const
{
  return type_;
}
const fs::path &ProjectSettings::getProjectRoot() const
{
  return project_root_;
}
