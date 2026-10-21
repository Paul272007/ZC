#include "core/project/target/ProjectCompiledTarget.hpp"

#include "core/project/LanguageConf.hpp"
#include "core/project/Project.hpp"
#include "framework/fs.hpp"

namespace zc::core
{

void ProjectCompiledTarget::loadFromJson(const json &conf)
{
  ProjectTarget::loadFromJson(conf);
  get_key_with_default(conf, "output", output_, name_);
  get_key_with_default(conf, "src_dirs", src_dirs_, { "src" });
}

Language ProjectCompiledTarget::linker(const SourceMap &sources)
{
  if (sources.contains(Language::CXX) && !sources.at(Language::CXX).empty())
    return Language::CXX;
  return Language::C;
}

SourceMap ProjectCompiledTarget::findSources() const
{
  SourceMap sources;
  bool      found_any = false;

  for (const auto &src_dir : src_dirs_) {
    const stdfs::path full_src_dir = root_ / src_dir;
    if (!fs::dir_exists(full_src_dir))
      throw_or_terminate(
        Exception(FSError::DirNotFound, "source directory {} does not exist", full_src_dir)
      );

    for (const auto &entry : stdfs::recursive_directory_iterator(full_src_dir)) {
      if (!entry.is_regular_file())
        continue;
      const stdfs::path &file = entry.path();

      if (const Language l = language_of(file); l != Language::Unknown) {
        stdfs::path rel_path = stdfs::relative(file, root_);
        sources[l].push_back(rel_path);
        found_any = true;
      }
    }
  }
  if (!found_any)
    throw_or_terminate(Exception(ZCError::NoSources, "no source files found for target: {}", name_));

  return sources;
}

} // namespace zc::core
