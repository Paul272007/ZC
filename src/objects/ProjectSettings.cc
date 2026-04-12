#include <filesystem>
#include <interface.hh>

#include <commands/Build.hh>
#include <fstream>
#include <helpers.hh>
#include <objects/ProjectSettings.hh>
#include <objects/ZCError.hh>
#include <string>
#include <tuple>

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
  version_ = Version(json_conf.value("version", "0.0.0"));
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
  output << root.dump(2);
  output.close();
}

void ProjectSettings::installPackage(std::filesystem::path &project_root, const bool force, const bool quiet)
{
  // Project settings of the package to install
  ProjectSettings p(project_root);

  if (p.getType() != LIB)
    throw ZCError();

  Build b(true, quiet, p);
  b.execute();

#ifdef DEBUG_MODE
  if (!quiet)
    debug("Projet compiled");
#endif

  fs::path local_zc = project_root_ / ZC_MODULES;
  fs::path dest_include = local_zc / "include" / p.getName();
  fs::path dest_lib = local_zc / "lib" / p.getName();

  if ((fs::exists(dest_include) || fs::exists(dest_lib) || pkgExists(p.getName())) && !force)
    if (!ask(
            "The library '" + p.getName() +
            "' seems to be already installed on this project. Do you want to reinstall it ?"
        ))
      return;

  if (!quiet)
    info("Installing headers...");

  fs::create_directories(dest_include);
  fs::copy(
      p.getIncludeFolder(), dest_include, fs::copy_options::recursive | fs::copy_options::overwrite_existing
  );

  if (!quiet)
    info("Installing libraries...");

  fs::create_directories(dest_lib);
  vector<string> lib_names = {p.getStaticLibName(), p.getSharedLibName()};

  for (const string &name : lib_names)
  {
    if (name.empty())
      continue;

    for (const auto &entry : fs::recursive_directory_iterator(project_root / "build"))
    {
      string filename = entry.path().filename().string();

      if (filename.find(name) != string::npos &&
          (entry.path().extension() == ".a" || entry.path().extension() == ".so" ||
           entry.path().extension() == ".dylib" || entry.path().extension() == ".lib"))
      {
        fs::copy_file(entry.path(), dest_lib / filename, fs::copy_options::overwrite_existing);
      }
    }
  }
  indexPackage(p.getName(), p.getVersion());
  success("Package " + p.getName() + " installed successfully.");
}

bool ProjectSettings::removePackage(const std::string &pkg_name)
{
  unindexPackage(pkg_name);
  fs::path include = project_root_ / ZC_MODULES / "include" / pkg_name;
  fs::path lib = project_root_ / ZC_MODULES / "lib" / pkg_name;

  if (!fs::exists(include) || !fs::exists(lib))
    return false;

  fs::remove_all(include);
  fs::remove_all(lib);
  return true;
}

bool ProjectSettings::pkgExists(const std::string &pkg_name) const
{
  const auto it = ranges::find_if(deps_, [&](const auto &d) { return std::get<0>(d) == pkg_name; });
  return it != deps_.end();
}

void ProjectSettings::indexPackage(const std::string &name, const Version &version)
{
  deps_.push_back(dependency(name, version));
  write();
}

void ProjectSettings::unindexPackage(const std::string &pkg_name)
{
  const auto it = ranges::find_if(deps_, [&](const auto &d) { return std::get<0>(d) == pkg_name; });

  if (it != deps_.end())
    deps_.erase(it);
  else
    throw ZCError(ZC_PACKAGE_NOT_FOUND, "The package was not found: " + pkg_name);

  write();
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
const dependencies &ProjectSettings::getDeps() const
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
