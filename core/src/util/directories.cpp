#include "core/util/directories.hpp"

#include "core/util/ZCError.hpp"
#include "framework/Exception.hpp"

namespace zc::core
{

const std::filesystem::path &zc_dir()
{
  static const stdfs::path root1 = [] {
    const char       *home  = getenv(user_home_env);
    const stdfs::path root2 = (home ? stdfs::path(home) : stdfs::current_path()) / zc_root_dir;

    if (!fs::dir_exists(root2)) {
      stdfs::create_directories(root2);
      // TODO: create default config here
    }

    return root2;
  }();
  return root1;
}

const stdfs::path &tmp_dir()
{
  static const stdfs::path tmp_dir = zc_dir() / zc_tmp_dir;
  return tmp_dir;
}

const stdfs::path &cache_dir()
{
  static const stdfs::path cache_dir = zc_dir() / zc_cache_dir;
  return cache_dir;
}

const stdfs::path &templates_dir()
{
  static const stdfs::path templates_dir = zc_dir() / zc_templates_dir;
  return templates_dir;
}

const stdfs::path &bin_links_dir()
{
  static const stdfs::path bin_links_dir = zc_dir() / zc_bin_dir;
  return bin_links_dir;
}

const stdfs::path &lib_links_dir()
{
  static const stdfs::path lib_links_dir = zc_dir() / zc_lib_dir;
  return lib_links_dir;
}

const stdfs::path &include_links_dir()
{
  static const stdfs::path include_links_dir = zc_dir() / zc_include_dir;
  return include_links_dir;
}

std::expected<stdfs::path, Exception> project_root_dir(const stdfs::path &path)
{
  if (!fs::dir_exists(path))
    return std::unexpected(Exception(FSError::DirNotFound, "directory {} not found", path));

  stdfs::path current = path;

  while (true) {
    if (fs::file_exists(current / zc_project_file))
      return current;

    if (current == current.root_path() || current == current.parent_path())
      return std::unexpected(Exception(ZCError::NotInAProject, "directory {} is not in a project", path));

    current = current.parent_path();
  }
}

bool in_a_project(const stdfs::path &path)
{
  return project_root_dir(path).has_value();
}

} // namespace zc::core
