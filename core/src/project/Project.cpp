#include "core/project/Project.hpp"

#include "core/global-config/GlobalConfig.hpp"
#include "core/project/Language.hpp"
#include "core/project/LanguageConf.hpp"
#include "core/registry/Registry.hpp"
#include "core/util/directories.hpp"
#include "core/util/ZCError.hpp"
#include "engine/Ctx.hpp"
#include "engine/engine.hpp"
#include "framework/shell.hpp"
#include "framework/util/string.hpp"

namespace zc::core
{

BuildMode Project::currentBuildMode() const
{
  return from_str<BuildMode>(fs::read(buildModeFile()).value_or("Auto")).value_or(BuildMode::Auto);
}

bool Project::isStructureValid() const
{
  return fs::file_exists(file_) && !name_.empty();
}

sh::Cmd Project::buildCommand(const u32 jobs, const std::string_view target) const
{
  sh::Cmd cmd({ "ninja", "-C" }, sh::Output::ShowAll);
  cmd << root_ << "-f" << ninjaFile();
  if (jobs > 0)
    cmd << "-j" << jobs;
  cmd << target;
  return cmd;
}

std::expected<void, Exception> Project::generateCompileCommands() const
{
#ifdef ZC_DEBUG
  sh::Cmd cmd({ "ninja", "-C" }, sh::Output::ShowAll);
#else
  sh::Cmd cmd({ "ninja", "-C" }, sh::Output::HideStderr);
#endif
  cmd << root_ << "-f" << ninjaFile() << "-t" << "compdb";

  std::string compdb;
  const int   code = cmd.forEachLine(4096, [&](const std::string &line) { compdb += line; });
  if (code != 0)
    return std::unexpected(
      Exception(ZCError::BuildFailed, "failed to generate compile_commands.json (exit code {})", code)
    );

  return fs::write(root_ / "compile_commands.json", compdb);
}

std::expected<void, Exception> Project::generateNinjaFile(BuildMode mode) const
{
  engine::Ctx ctx;
  ctx.set("AR", std::string(gc().archiver()));

  bool has_cxx = false;
  bool has_c   = false;

  for (const auto &[lang, conf] : languages_) {
    const std::string name(to_str(lang).value_or("C"));

    if (lang == Language::CXX)
      has_cxx = true;
    else if (lang == Language::C)
      has_c = true;

    std::vector<std::string> flags = conf.flags;
    switch (mode) {
    case BuildMode::Debug:
      flags.emplace_back("-g");
      flags.emplace_back("-Og");
      break;
    case BuildMode::Release:
      flags.emplace_back("-O3");
      break;
    case BuildMode::ReleaseWithDebugInfo:
      flags.emplace_back("-O2");
      flags.emplace_back("-g");
      break;
    default:
      break;
    }

    std::string compiler = conf.compiler.empty() ? gc().languages().at(lang).compiler : conf.compiler;
    std::string std_ver  = conf.std.empty() ? gc().languages().at(lang).std : conf.std;

    ctx.set(name + "_COMPILER", compiler);
    ctx.set(name + "_FLAGS", join(flags, " "));
    ctx.set(name + "_STD", std_ver);
  }

  ctx.set("HAS_CXX", has_cxx);
  ctx.set("HAS_C", has_c);

  std::vector<std::string> all_statements;
  std::vector<std::string> all_outputs;

  for (const auto &target : targets_) {
    auto stmts = target->generateBuildStatements();
    all_statements.insert(all_statements.end(), stmts.begin(), stmts.end());

    for (const auto &out : target->outputFiles()) {
      std::error_code ec;
      auto            rel = stdfs::relative(out, root_, ec);
      if (!ec && !rel.empty())
        all_outputs.push_back(rel.string());
      else
        all_outputs.push_back(out.string());
    }
  }

  ctx.setList("BUILD_STATEMENTS", all_statements);
  ctx.setList("ALL_OUTPUTS", all_outputs);

  return engine::render_to(templates_dir() / zc_ninja_template_file, ninjaFile(), ctx);
}

void Project::load()
{
  if (!fs::file_exists(file_))
    throw_or_terminate(Exception(FSError::FileNotFound, "Project does not exist"));

  const auto read_res = fs::read_json(file_);
  if (!read_res)
    throw_or_terminate(read_res.error());
  const auto &json_conf = read_res.value();

  get_key(json_conf, "name", name_);
  get_optional_key(json_conf, "author", author_);
  get_optional_key(json_conf, "description", description_);
  get_key(json_conf, "version", version_);

  if (jsonContainsObject(json_conf, "languages"))
    for (const auto &[lang, conf] : json_conf["languages"].items())
      if (const auto l = from_str<Language>(lang)) {
        LanguageConf lang_conf = conf.get<LanguageConf>();

        lang_conf.language = *l;

        if (lang_conf.compiler.empty())
          lang_conf.compiler = gc().languages().at(*l).compiler;

        if (lang_conf.flags.empty())
          lang_conf.flags = gc().languages().at(*l).flags;

        if (lang_conf.std.empty())
          lang_conf.std = gc().languages().at(*l).std;

        languages_.insert_or_assign(*l, std::move(lang_conf));
      }

  if (languages_.empty())
    languages_.insert_or_assign(Language::CXX, gc().languages().at(Language::CXX));

  if (jsonContainsArray(json_conf, "targets")) {
    for (const auto &target : json_conf["targets"])
      targets_.push_back(ProjectTarget::fromRoot(root_ / target.get<std::string_view>(), this));
  } else {
    // Load target from root directory
    targets_.push_back(ProjectTarget::fromJson(root_, json_conf, this));
  }

  for (auto &target : targets_)
    target->resolveInternalDeps(targets_);
}

void Project::write()
{
  json j;
  j["name"]        = name_;
  j["author"]      = author_;
  j["description"] = description_;
  j["version"]     = version_;

  json langs = json::object();
  for (const auto &[l, conf] : languages_)
    if (auto name = to_str(l))
      langs[*name] = conf;
  j["languages"] = langs;

  auto write_res = fs::write_json(file_, j);
  if (!write_res)
    throw_or_terminate(write_res.error());
  modified_ = false;
}

Project::Project(stdfs::path root) : Conf(root / zc_project_file), root_(std::move(root))
{
  load();
}

std::expected<void, Exception> Project::installDependencies(const u32 jobs) const
{
  for (const auto &target : targets_) {
    for (const auto &dep : target->externalDeps()) {
      if (reg().isInstalled(dep.package, dep.version))
        continue;
      auto install_res = reg().installFromServer(dep.package, dep.version, jobs, false);
      if (!install_res)
        return std::unexpected(install_res.error());
    }
  }
  return {};
}

std::expected<void, Exception> Project::updateDependencies(const u32 jobs) const
{
  for (const auto &target : targets_) {
    for (const auto &dep : target->externalDeps()) {
      auto install_res = reg().updateFromServer(dep.package, dep.version, jobs);
      if (!install_res)
        return std::unexpected(install_res.error());
    }
  }
  return {};
}

std::expected<void, Exception> Project::uninstallDependencies() const
{
  for (const auto &target : targets_) {
    for (const auto &dep : target->externalDeps()) {
      auto install_res = reg().uninstall(dep.package, dep.version, false);
      if (!install_res)
        return std::unexpected(install_res.error());
    }
  }
  return {};
}

std::expected<void, Exception> Project::useDependencies() const
{
  for (const auto &target : targets_) {
    for (const auto &dep : target->externalDeps()) {
      auto use_res = reg().setDefaultVersion(dep.package, dep.version);
      if (!use_res)
        return std::unexpected(use_res.error());
    }
  }
  return {};
}

void Project::clean() const
{
  if (fs::dir_exists(buildDir())) {
    std::error_code ec;
    stdfs::remove_all(buildDir(), ec);
  }
}

std::expected<void, Exception>
Project::generateBuildConfig(BuildMode mode, const bool compile_commands) const
{
  const BuildMode current_mode = currentBuildMode();

  if (mode == BuildMode::Auto)
    if (current_mode == BuildMode::Auto)
      mode = BuildMode::Debug; // default mode
    else
      mode = current_mode;
  else if (current_mode != BuildMode::Auto && mode != current_mode)
    clean(); // mode changed -> force full rebuild

  std::error_code ec;
  stdfs::create_directories(buildDir(), ec);
  auto _ = fs::write(buildModeFile(), std::string(to_str(mode).value_or("Debug")));

  if (auto ninja_res = generateNinjaFile(mode); !ninja_res)
    return ninja_res;

  if (compile_commands)
    return generateCompileCommands();

  return {};
}

std::expected<void, Exception>
Project::buildTarget(const std::string_view target, const BuildMode mode, const u32 jobs) const
{
  bool found = false;
  for (const auto &t : targets_) {
    if (t->name() == target) {
      found = true;
      break;
    }
  }
  if (!found)
    return std::unexpected(Exception(ZCError::TargetNotFound, "target '{}' not found in project", target));

  auto config_res = generateBuildConfig(mode);
  if (!config_res)
    return config_res;

  const sh::Cmd cmd = buildCommand(jobs, target);

  const int code = cmd();
  if (code != 0)
    return std::unexpected(Exception(ZCError::BuildFailed, "ninja build failed with exit code {}", code));

  return {};
}

std::expected<void, Exception> Project::buildAll(const BuildMode mode, const u32 jobs) const
{
  auto config_res = generateBuildConfig(mode);
  if (!config_res)
    return config_res;

  const int code = buildCommand(jobs, "all")();
  if (code != 0)
    return std::unexpected(Exception(ZCError::BuildFailed, "ninja build failed with exit code {}", code));

  return {};
}

std::expected<void, Exception> Project::installBuild(const u32 jobs) const
{
  auto config_res = generateBuildConfig(BuildMode::Release, false);
  if (!config_res)
    return config_res;

  const int code = buildCommand(jobs, "all")();
  if (code != 0)
    return std::unexpected(Exception(ZCError::BuildFailed, "ninja build failed with exit code {}", code));

  return {};
}

std::expected<void, Exception> Project::installTargets() const
{
  for (const auto &target : targets_) {
    auto res = target->install();
    if (!res)
      return res;
  }
  return {};
}

} // namespace zc::core
