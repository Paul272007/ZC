#include "core/project/target/ProjectBinTarget.hpp"

#include "core/util/ZCError.hpp"

namespace zc::core
{

stdfs::path ProjectBinTarget::installDir() const
{
  return cache_dir() / parent_->name() / parent_->version().string() / zc_bin_dir;
}

std::string ProjectBinTarget::binFmt() const
{
#ifdef ZC_WINDOWS
  return std::format("{}.exe", output_);
#else
  return output_;
#endif
}

void ProjectBinTarget::load()
{
  ProjectTarget::load();
}

void ProjectBinTarget::write() {}

bool ProjectBinTarget::isOutputValid() const
{
  return !output_.empty() && output_.find_first_of("/\\:") == std::string::npos;
}

std::unique_ptr<RegistryTarget> ProjectBinTarget::toRegistryTarget() const
{
  std::vector<std::string> internal_deps;
  internal_deps.reserve(internal_deps_.size());
  for (const auto &dep : internal_deps_)
    internal_deps.push_back(dep->name());

  std::vector<RegistryTargetDependency> external_deps;
  external_deps.reserve(external_deps_.size());
  for (const auto &dep : external_deps_)
    external_deps.push_back(dep.toRegistryTargetDependency());

  auto target           = std::make_unique<RegistryBinTarget>();
  target->name          = name_;
  target->output        = output_;
  target->inc_dirs      = public_inc_dirs_;
  target->macros        = public_macros_;
  target->internal_deps = std::move(internal_deps);
  target->external_deps = std::move(external_deps);
  return target;
}

std::expected<void, Exception> ProjectBinTarget::build()
{
  if (!parent_)
    return std::unexpected(Exception(ZCError::TargetNotFound, "target '{}' has no parent project", name_));
  return parent_->buildTarget(name_);
}

std::expected<void, Exception> ProjectBinTarget::install()
{
  const stdfs::path dest = installDir() / binFmt();
  const stdfs::path src  = outputFiles().front();

  if (!fs::file_exists(src))
    return std::unexpected(Exception(FSError::FileNotFound, "expected compiled target not found: {}", src));

  std::error_code ec;
  stdfs::create_directories(installDir(), ec);
  stdfs::copy(src, dest, stdfs::copy_options::overwrite_existing, ec);
  if (ec)
    return std::unexpected(
      Exception(FSError::WritingError, "failed to install binary to {}: {}", dest, ec.message())
    );
  return {};
}

std::vector<stdfs::path> ProjectBinTarget::outputFiles() const
{
  return { parent_->buildDir() / name_ / binFmt() };
}

std::vector<std::string> ProjectBinTarget::generateBuildStatements() const
{
  std::vector<std::string> statements;
  const SourceMap          sources = findSources();
  const auto               flags   = resolveFlags();

  const std::string includes_str = join(flags.includes, " ");
  const std::string macros_str   = join(flags.macros, " ");

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
      stdfs::path     full_src = root_ / src_rel_to_target;
      std::error_code ec;
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

  const Language link_lang  = linker(sources);
  std::string    linker_cmd = "clang++";
  if (parent_ && parent_->languages().contains(link_lang))
    linker_cmd = parent_->languages().at(link_lang).compiler;

  stdfs::path       out_rel     = stdfs::path(zc_build_dir) / name_ / binFmt();
  const std::string ldflags_str = join(flags.ldflags, " ");

  std::string link_stmt = std::format(
    "build {}: link {}\n" "  linker = {}\n" "  ldflags = {}", out_rel.string(), join(all_objs, " "),
    linker_cmd, ldflags_str
  );
  statements.push_back(std::move(link_stmt));

  statements.push_back(std::format("build {}: phony {}", name_, out_rel.string()));

  return statements;
}

int ProjectBinTarget::execute(const std::vector<std::string> &args, const sh::Output output) const
{
  sh::Cmd cmd({ outputFiles().front().string() }, output);
  for (const auto &arg : args)
    cmd << arg;

  return cmd();
}

} // namespace zc::core
