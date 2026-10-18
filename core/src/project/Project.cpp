#include "core/project/Project.hpp"

#include "core/util/directories.hpp"

namespace zc::core
{

const stdfs::path &Project::buildDir() const
{
  static const stdfs::path build_dir = root_ / zc_build_dir;
  return build_dir;
}

} // namespace zc::core
