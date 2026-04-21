#include <filesystem>

#include <commands/Clean.hh>

namespace fs = std::filesystem;

Clean::Clean(const bool force, const bool quiet, const std::filesystem::path &project_root)
    : Command(force, quiet), root_(project_root), p_settings_(ProjectSettings(root_))
{
}

int Clean::operator()()
{
  if (fs::exists(root_ / "CMakeLists.txt"))
    if (fs::remove(root_ / "CMakeLists.txt"))
      log_info("Cleaned CMakeLists.txt");

  if (fs::exists(root_ / ".cache"))
    if (fs::remove_all(root_ / ".cache") > 0)
      log_info("Cleaned .cache/");

  if ((p_settings_.type_ == BIN || !force_) && fs::exists(root_ / "build"))
    if (fs::remove_all(root_ / "build") > 0)
      log_info("Cleaned build/");

  return 0;
}
