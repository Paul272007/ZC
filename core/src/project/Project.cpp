#include "core/project/Project.hpp"

#include "core/project/LanguageConf.hpp"
#include "core/project/target/ProjectBinTarget.hpp"
#include "core/util/directories.hpp"
#include "core/util/helpers.hpp"
#include "core/util/ZCError.hpp"
#include "engine/Ctx.hpp"
#include "engine/engine.hpp"
#include "framework/shell.hpp"
#include "framework/util/string.hpp"

namespace zc::core
{

bool Project::isStructureValid() const
{
  return fs::file_exists(file_) && !name_.empty();
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
  get_key(json_conf, "author", author_);
  get_key(json_conf, "description", description_);
  get_key(json_conf, "version", version_);

  if (jsonContainsObject(json_conf, "languages"))
    for (const auto &[lang, conf] : json_conf["languages"].items())
      if (const auto l = from_str<Language>(lang)) {
        LanguageConf lang_conf;
        lang_conf.language = *l;
        conf.get_to(lang_conf);
        languages_.insert_or_assign(*l, std::move(lang_conf));
      }

  if (languages_.empty()) {
    LanguageConf cxx_conf;
    cxx_conf.language         = Language::CXX;
    cxx_conf.compiler         = default_compiler(Language::CXX);
    cxx_conf.std              = default_std(Language::CXX);
    cxx_conf.flags            = default_flags(Language::CXX);
    languages_[Language::CXX] = std::move(cxx_conf);
  }

  if (jsonContainsArray(json_conf, "targets")) {
    for (const auto &target : json_conf["targets"])
      targets_.push_back(ProjectTarget::fromRoot(root_ / target.get<std::string_view>(), this));
  } else {
    // Load target from root directory
    targets_.push_back(ProjectTarget::fromJson(root_, json_conf, this));
  }
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

void Project::clean() const
{
  if (fs::dir_exists(buildDir())) {
    std::error_code ec;
    stdfs::remove_all(buildDir(), ec);
  }
}

BuildMode Project::currentBuildMode() const
{
  return from_str<BuildMode>(fs::read(buildModeFile()).value_or("Auto")).value_or(BuildMode::Auto);
}

std::expected<void, Exception> Project::generateBuildConfig(BuildMode mode)
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

  return generateBuildFile(mode);
}

std::expected<void, Exception> Project::generateBuildConfigInstall(BuildMode mode)
{
  if (mode == BuildMode::Auto)
    mode = BuildMode::Release;
  return generateBuildConfig(mode);
}

std::expected<void, Exception> Project::generateCompileCommands(BuildMode /*mode*/)
{
  return {};
}

std::expected<void, Exception> Project::generateBuildFile(BuildMode mode)
{
  engine::Ctx ctx;
  ctx.set("AR", "ar");

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

    std::string compiler = conf.compiler.empty() ? default_compiler(lang) : conf.compiler;
    std::string std_ver  = conf.std.empty() ? default_std(lang) : conf.std;

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

std::expected<void, Exception>
Project::buildTarget(std::string_view target, const BuildMode mode, const u32 jobs)
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

  sh::Cmd cmd({ "ninja", "-C" }, sh::Output::ShowAll);
  cmd << root().string() << "-f" << ninjaFile().string();
  if (jobs > 0)
    cmd << "-j" << jobs;
  cmd << target;

  const int code = cmd();
  if (code != 0)
    return std::unexpected(Exception(ZCError::BuildFailed, "ninja build failed with exit code {}", code));

  return {};
}

std::expected<void, Exception> Project::buildAll(const BuildMode mode, const u32 jobs)
{
  auto config_res = generateBuildConfig(mode);
  if (!config_res)
    return config_res;

  sh::Cmd cmd({ "ninja", "-C" }, sh::Output::ShowAll);
  cmd << root().string() << "-f" << ninjaFile().string();
  if (jobs > 0)
    cmd << "-j" << jobs;
  cmd << "all";

  const int code = cmd();
  if (code != 0)
    return std::unexpected(Exception(ZCError::BuildFailed, "ninja build failed with exit code {}", code));

  return {};
}

std::expected<void, Exception> Project::installBuild(u32 jobs)
{
  auto build_res = buildAll(BuildMode::Release, jobs);
  if (!build_res)
    return build_res;

  return installTargets();
}

std::expected<void, Exception> Project::installTargets()
{
  for (const auto &target : targets_) {
    auto res = target->install();
    if (!res)
      return res;
  }
  return {};
}

std::expected<void, Exception> Project::installDependencies()
{
  return {};
}

std::expected<void, Exception> Project::updateDependencies()
{
  return {};
}

} // namespace zc::core
