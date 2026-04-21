#include <filesystem>
#include <string>
#include <vector>

#include <helpers.hh>
#include <objects/ProjectSettings.hh>
#include <objects/Settings.hh>
#include <objects/ZCError.hh>

using json = nlohmann::json;
using namespace std;
namespace fs = std::filesystem;

ProjectSettings::ProjectSettings(const std::filesystem::path &project_root) : Settings(project_root)
{
  load();
  checkFolderNames();
}

ProjectSettings::ProjectSettings(
    const std::string &name, const std::string &author, const std::string &targetName, const bool add_std,
    const std::string &version, const std::string &src, const std::string &include, const ProjectType &type
)
    : Settings(fs::current_path()), type_(type), name_(name), author_(author), target_name_(targetName),
      version_(version), src_folder_(src), include_folder_(include)
{
  // If the project is being created (and does not exist yet), the root is the current path and zc.json
  // doesn't exist yet
  add_std_ = add_std;
  checkFolderNames();
}

void ProjectSettings::load()
{
  json json_conf = parseJsonFile(config_path_);

  if (!json_conf.contains("name") || !json_conf.contains("target"))
    throw ZCError(ZC_CONFIG_MISSING_PROPERTY, "required properties for project are missing");

  static_compile_ = json_conf.value<bool>("static", false);
  name_ = json_conf.value("name", "");
  author_ = json_conf.value("author", "");
  c_std_ = json_conf.value("c_std", "c17");
  cpp_std_ = json_conf.value("cpp_std", "c++20");
  add_std_ = json_conf.value("add_std", false);
  version_ = Version(json_conf.value("version", "0.0.0"));
  src_folder_ = root_ / json_conf.value("srcFolder", "src");
  include_folder_ = root_ / json_conf.value("includeFolder", "include");
  target_name_ = json_conf.value("target", "");

  // Type
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

  writeJsonFile(root, config_path_);
}

void ProjectSettings::checkFolderNames() const
{
  vector<std::string> invalid_names{"external",       "build",         ".cache",
                                    "CMakeLists.txt", "registry.json", "zc.json"};

  std::string src_str = src_folder_.filename().string();
  std::string inc_str = include_folder_.filename().string();

  bool is_src_invalid = std::ranges::find(invalid_names, src_str) != invalid_names.end();
  bool is_inc_invalid = std::ranges::find(invalid_names, inc_str) != invalid_names.end();

  if (is_src_invalid)
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "Source folder has a forbidden name");

  if (is_inc_invalid)
    throw ZCError(ZC_CONFIG_CONTENT_ERROR, "Include folder has a forbidden name");
}
