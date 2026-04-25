#include <filesystem>

#include "commands/Init.hh"
#include "files.hh"
#include "nlohmann/json.hpp"
#include "objects/Configs/LocalConfig.hh"
#include "objects/ZCError.hh"

using namespace std;
namespace fs = std::filesystem;

Init::Init(
    bool force, bool quiet, bool edit, bool git, const std::string &path, const std::string &author,
    const std::string &project_template, const std::string &name, const std::string &type
)
    : Command(force, quiet), edit_(edit), git_(git), author_(author), template_(project_template),
      name_(name), path_(path.empty() ? fs::current_path() : fs::path(path)), type_(Type::UNDEF),
      g_(logger_, force)
{
  if (!force_ && fs::exists(CONFIG))
    if (!ask(
            "It seems like a ZC project is already initialized in this directory. Do you want to overwrite "
            "it ?"
        ))
      exit(0);

  // Ask if not precised (default option for the package is the current directory)
  if (name_.empty())
    name_ = input("Package name: ", fs::current_path().filename().string());

  target_ = input("Package target: ", fs::current_path().filename().string());

  if (author_.empty())
    author_ = input("Project author: ", g_.gc_->default_author_);

  if (template_.empty())
    template_ = input("Template to use to initialize project: ");

  // std::string type = input("Project type: (lib/bin)");
  if (!type.empty())
  {
    std::string type2 = upper(type);
    if (type2 == "LIB")
      type_ = Type::LIB;
    else if (type2 == "BIN")
      type_ = Type::BIN;
  }
  // else it stays UNDEF
}

int Init::operator()()
{
  if (!template_.empty())
    g_.initializeWithTemplate(path_, template_);

  if (git_)
  {
    logger_(LogLevel::INFO, "Initializing git repo...");
    if (system("git init") != 0)
      throw ZCError(ZC_GIT_ERROR, "Git init failed");
  }

  nlohmann::json base_config;
  base_config["name"] = name_;
  base_config["author"] = author_;
  base_config["target"] = target_;
  base_config["add_std"] = g_.gc_->add_std_;
  base_config["type"] = type_ == Type::LIB ? "lib" : (type_ == Type::BIN ? "bin" : "undef");
  writeJsonFile(base_config, fs::current_path() / CONFIG);

  if (g_.gc_->edit_on_init_ || edit_)
    return system(string(g_.gc_->editor_ + " " + path_.string()).c_str());

  return 0;
}
