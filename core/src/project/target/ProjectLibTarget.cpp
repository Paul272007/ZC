#include "core/project/target/ProjectLibTarget.hpp"

#include "core/project/Project.hpp"
#include "core/registry/target/RegistryLibTarget.hpp"
#include "core/util/helpers.hpp"

namespace zc::core
{

std::string ProjectLibTarget::sharedFmt() const
{
#ifdef ZC_WINDOWS
  return std::format("{}-{}.dll", parent_->name(), output_);
#elifdef ZC_MACOS
  return std::format("lib{}-{}.dylib", parent_->name(), output_);
#else
  return std::format("lib{}-{}.so", parent_->name(), output_);
#endif
}

std::string ProjectLibTarget::staticFmt() const
{
#ifdef ZC_WINDOWS
  return std::format("{}-{}.a", parent_->name(), output_);
#else
  return std::format("lib{}-{}.a", parent_->name(), output_);
#endif
}

std::expected<void, Exception> ProjectLibTarget::installLibs() const
{
  const stdfs::path static_src  = outputFiles().at(0);
  const stdfs::path shared_src  = outputFiles().at(1);
  const stdfs::path static_dest = installDir() / zc_lib_dir / staticFmt();
  const stdfs::path shared_dest = installDir() / zc_lib_dir / sharedFmt();

  if (!fs::file_exists(static_src))
    return std::unexpected(
      Exception(FSError::FileNotFound, "expected static target not found: {}", static_src)
    );
  if (!fs::file_exists(shared_src))
    return std::unexpected(
      Exception(FSError::FileNotFound, "expected shared target not found: {}", shared_src)
    );

  std::error_code ec;
  stdfs::create_directories(installDir() / zc_lib_dir, ec);
  stdfs::copy_file(static_src, static_dest, stdfs::copy_options::overwrite_existing, ec);
  stdfs::copy_file(shared_src, shared_dest, stdfs::copy_options::overwrite_existing, ec);
  if (ec)
    return std::unexpected(Exception(
      FSError::CopyError, "failed to install libraries to {}: {}", installDir() / zc_lib_dir, ec.message()
    ));
  return {};
}

std::expected<void, Exception> ProjectLibTarget::install() const
{
  const auto libs_res = installLibs();
  if (!libs_res)
    return std::unexpected(libs_res.error());
  return installHeaders();
}

std::vector<std::string> ProjectLibTarget::generateBuildStatements() const
{
  std::vector<std::string> statements;
  const SourceMap          sources  = findSources();
  const auto               flags    = resolveFlags();
  const auto               all_objs = compileObjectStatements(sources, flags, statements, true);

  const std::string objs_str   = join(all_objs, " ");
  const stdfs::path static_rel = stdfs::path(zc_build_dir) / name_ / staticFmt();
  const stdfs::path shared_rel = stdfs::path(zc_build_dir) / name_ / sharedFmt();

  statements.push_back(std::format("build {}: archive {}", static_rel.string(), objs_str));
  statements.push_back(formatLinkStatement(shared_rel, all_objs, sources, flags, true));
  statements.push_back(
    std::format("build {}: phony {} {}", name_, static_rel.string(), shared_rel.string())
  );

  return statements;
}

std::vector<stdfs::path> ProjectLibTarget::outputFiles() const
{
  return { parent_->buildDir() / name_ / staticFmt(), parent_->buildDir() / name_ / sharedFmt() };
}

std::unique_ptr<RegistryTarget> ProjectLibTarget::whenInstalled() const
{
  auto target = std::make_unique<RegistryLibTarget>();
  fillRegistryTarget(*target);
  target->static_output = staticFmt();
  target->shared_output = sharedFmt();
  return target;
}

} // namespace zc::core
