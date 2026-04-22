#include "commands/Init.hh"
#include "objects/Controller.hh"
#include "objects/LocalConfig.hh"
#include "objects/ZCError.hh"

using namespace std;
namespace fs = std::filesystem;

Init::Init(
    bool force, bool quiet, bool edit, bool git, const std::string &author,
    const std::string &project_template, const std::string &name, const std::string &type
)
    : Command(force, quiet), path_(fs::current_path()), type_(Type::UNDEF), edit_(edit), git_(git),
      l_(logger_, force), g_(logger_, force)
{
  if (!force_ && fs::exists(CONFIG))
    if (!ask(
            "It seems like a ZC project is already initialized in this directory. Do you want to overwrite "
            "it ?"
        ))
      exit(0);

  // Ask if not precised (default option for the package is the current directory)
  if (name.empty())
    name_ = input("Package name: ", fs::current_path().filename().string());

  target_ = input("Package target: ", fs::current_path().filename().string());

  if (author.empty())
    author_ = input("Project author: ");

  if (project_template.empty())
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
  {
    g_.initializeWithTemplate(l_.root_dir_, template_);
  }

  if (git_)
  {
    logger_(LogLevel::INFO, "Initializing git repo...");
    if (system("git init") != 0)
      throw ZCError(ZC_GIT_ERROR, "Git init failed");
  }

  // Create configuration file with empty version
  l_.lc_->name_ = name_;
  l_.lc_->author_ = author_;
  l_.lc_->target_ = target_;
  l_.lc_->add_std_ = g_.gc_->add_std_;
  l_.lc_->type_ = type_;
  l_.lc_->write();

  if (g_.gc_->edit_on_init_ || edit_)
    return system(string(g_.gc_->editor_ + " " + path_.string()).c_str());

  return 0;
}
