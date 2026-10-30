#include "core/project/target/ProjectBinTarget.hpp"

#include <string_view>

namespace zc::core
{

std::string ProjectBinTarget::binFmt() const
{
#ifdef ZC_WINDOWS
  return std::format("{}.exe", output_);
#else
  return output_;
#endif
}

std::unique_ptr<RegistryTarget> ProjectBinTarget::whenInstalled() const
{
  auto target = std::make_unique<RegistryBinTarget>();
  fillRegistryTarget(*target);
  target->output = binFmt();
  return target;
}

std::expected<void, Exception> ProjectBinTarget::install() const
{
  const stdfs::path src  = outputFiles().front();
  const stdfs::path dest = installDir() / zc_bin_dir / binFmt();

  if (!fs::file_exists(src))
    return std::unexpected(Exception(FSError::FileNotFound, "expected compiled target not found: {}", src));

  std::error_code ec;
  stdfs::create_directories(installDir() / zc_bin_dir, ec);
  stdfs::copy_file(src, dest, stdfs::copy_options::overwrite_existing, ec);
  if (ec)
    return std::unexpected(
      Exception(FSError::CopyError, "failed to install binary to {}: {}", dest, ec.message())
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
  const SourceMap          sources  = findSources();
  const auto               flags    = resolveFlags();
  const auto               all_objs = compileObjectStatements(sources, flags, statements, false);

  const stdfs::path out_rel = stdfs::path(zc_build_dir) / name_ / binFmt();
  statements.push_back(formatLinkStatement(out_rel, all_objs, sources, flags, false));
  statements.push_back(std::format("build {}: phony {}", name_, out_rel.string()));

  return statements;
}

int ProjectBinTarget::execute(const std::vector<std::string_view> &args, const sh::Output output) const
{
  sh::Cmd cmd({ outputFiles().front().string() }, output);
  for (const auto &arg : args)
    cmd << arg;

  return cmd();
}

} // namespace zc::core
