#include <string>

#include "commands/Build.hh"
#include "helpers.hh"
#include "objects/Controllers/Controller.hh"

using namespace std;
namespace fs = std::filesystem;

Build::Build(
    const bool force, const bool quiet, const bool clean, const bool release, const std::string &path
)
    : Command(force, quiet), clean_(clean), release_(release), path_(path),
      l_(logger_, force, path_.empty() ? getProjectRoot() : getProjectRoot(path)), g_(logger_, force)
{
}

int Build::operator()()
{
  l_.buildProject(quiet_, release_);

  if (g_.gc_->move_binary_to_current_path_ && l_.lc_->type_ == Type::BIN)
  {
    fs ::path binary = l_.root_dir_ / BUILD_DIR / l_.lc_->target_;
    if (fs::exists(binary))
      fs::rename(binary, fs::current_path() / l_.lc_->target_);
  }

  if (clean_)
    l_.cleanProject();

  return 0;
}
