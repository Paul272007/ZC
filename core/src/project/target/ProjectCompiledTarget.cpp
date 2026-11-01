#include "core/project/target/ProjectCompiledTarget.hpp"

#include "core/global-config/GlobalConfig.hpp"
#include "core/project/Project.hpp"
#include "core/util/helpers.hpp"
#include "framework/fs.hpp"

namespace zc::core
{

void ProjectCompiledTarget::loadFromJson(const json &conf)
{
  ProjectTarget::loadFromJson(conf);
  get_key_with_default(conf, "output", output_, name_);
  get_key_with_default(conf, "src_dirs", src_dirs_, { "src" });
}

void ProjectCompiledTarget::writeToJson(json &conf) const
{
  ProjectTarget::writeToJson(conf);
  conf["output"]   = output_;
  conf["src_dirs"] = src_dirs_;
}

bool ProjectCompiledTarget::isOutputValid() const
{
  // TODO: add other rules
  return !output_.empty() && output_.find_first_of("/\\:") == std::string::npos;
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

std::vector<std::string> ProjectCompiledTarget::compileObjectStatements(
  const SourceMap &sources, const ProjectTargetResolvedFlags &flags, std::vector<std::string> &statements,
  const bool pic
) const
{
  const std::string includes_str = join(flags.includes, " ");
  std::string       macros_str   = join(flags.macros, " ");
#ifndef ZC_WINDOWS
  if (pic)
    macros_str = macros_str.empty() ? "-fPIC" : macros_str + " -fPIC";
#else
  (void)pic;
#endif

  std::vector<std::string> all_objs;

  for (const auto &[lang, src_files] : sources) {
    std::string rule_name;
    if (lang == Language::CXX)
      rule_name = "compile_cxx";
    else if (lang == Language::C)
      rule_name = "compile_c";
    else
      continue;

    for (const auto &src_rel_to_target : src_files) {
      std::error_code ec;
      stdfs::path     full_src        = root_ / src_rel_to_target;
      stdfs::path     src_rel_to_proj = stdfs::relative(full_src, parent_->root(), ec);
      if (ec || src_rel_to_proj.empty())
        src_rel_to_proj = full_src;

      stdfs::path obj_rel = stdfs::path(zc_build_dir) / name_ / "obj" / (src_rel_to_target.string() + ".o");
      all_objs.push_back(obj_rel.string());

      std::string stmt = std::format(
        "build {}: {}\n" "  includes = {}\n" "  macros = {}", obj_rel.string(),
        rule_name + " " + src_rel_to_proj.string(), includes_str, macros_str
      );
      statements.push_back(std::move(stmt));
    }
  }

  return all_objs;
}

std::string ProjectCompiledTarget::resolveLinker(const SourceMap &sources) const
{
  const Language link_lang = linker(sources);
  if (parent_ && parent_->languages().contains(link_lang))
    return parent_->languages().at(link_lang).compiler;
  return gc().languages().at(link_lang).compiler;
}

std::string ProjectCompiledTarget::formatLinkStatement(
  const stdfs::path &out_rel, const std::vector<std::string> &objs, const SourceMap &sources,
  const ProjectTargetResolvedFlags &flags, const bool shared
) const
{
  std::string inputs = join(objs, " ");
  if (!flags.implicit_deps.empty())
    inputs += " | " + join(flags.implicit_deps, " ");

  std::vector<std::string> all_ldflags;
  if (shared)
    all_ldflags.emplace_back("-shared");
  all_ldflags.insert(all_ldflags.end(), flags.static_libs.begin(), flags.static_libs.end());
  all_ldflags.insert(all_ldflags.end(), flags.ldflags.begin(), flags.ldflags.end());

  return std::format(
    "build {}: link {}\n" "  linker = {}\n" "  ldflags = {}", out_rel.string(), inputs,
    resolveLinker(sources), join(all_ldflags, " ")
  );
}

ProjectTargetResolvedFlags ProjectCompiledTarget::resolveFlags() const
{
  auto flags = ProjectTarget::resolveFlags();
  for (const auto &src_dir : src_dirs_)
    flags.includes.push_back(formatIncludeFlag(root_ / src_dir));
  return flags;
}

} // namespace zc::core
