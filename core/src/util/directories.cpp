#include "core/util/directories.hpp"

namespace zc::core
{

const std::filesystem::path &zc_root()
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
  static const stdfs::path tmp_dir = zc_root() / zc_tmp_dir;
  return tmp_dir;
}

const stdfs::path &cache_dir()
{
  static const stdfs::path cache_dir = zc_root() / zc_cache_dir;
  return cache_dir;
}

const stdfs::path &bin_links_dir()
{
  static const stdfs::path bin_links_dir = zc_root() / zc_bin_dir;
  return bin_links_dir;
}

const stdfs::path &lib_links_dir()
{
  static const stdfs::path lib_links_dir = zc_root() / zc_lib_dir;
  return lib_links_dir;
}

const stdfs::path &include_links_dir()
{
  static const stdfs::path include_links_dir = zc_root() / zc_include_dir;
  return include_links_dir;
}

} // namespace zc::core
