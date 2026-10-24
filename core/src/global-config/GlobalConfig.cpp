#include "core/global-config/GlobalConfig.hpp"

#include <string>
#include <sys/stat.h>

#include "core/project/Language.hpp"
#include "core/project/LanguageConf.hpp"
#include "core/util/directories.hpp"
#include "core/util/helpers.hpp"
#include "core/util/json_helpers.hpp"
#include "framework/fs.hpp"
#include "framework/ui/UI.hpp"
#include "framework/util/helpers.hpp"

namespace zc::core
{

GlobalConfig::GlobalConfig() : Conf(zc_dir() / zc_config_file)
{
  if (!fs::file_exists(file_))
    modified_ = true;
  else
    GlobalConfig::load();

  if (!languages_.contains(Language::C)) {
    languages_.insert_or_assign(
      Language::C,
      LanguageConf{
        .language = Language::C,
        .std      = std::string(default_std(Language::C)),
        .compiler = std::string(default_compiler(Language::C)),
        .flags    = default_flags(Language::C),
      }
    );
  }
  if (!languages_.contains(Language::CXX)) {
    languages_.insert_or_assign(
      Language::CXX,
      LanguageConf{
        .language = Language::CXX,
        .std      = std::string(default_std(Language::CXX)),
        .compiler = std::string(default_compiler(Language::CXX)),
        .flags    = default_flags(Language::CXX),
      }
    );
  }
}

GlobalConfig::~GlobalConfig()
{
  if (modified_)
    try {
      GlobalConfig::write();
    } catch (const std::exception &e) {
      ui.debug("{}", e.what());
    }
}

void GlobalConfig::load()
{
  const auto res = fs::read_json(file_);
  if (!res)
    return;
  const auto &j = *res;

  const auto get_flag = [&j](const std::string_view key, const bool current) {
    bool value = current;
    get_optional_key(j, key, value);
    return value;
  };

  always_keep_       = get_flag("always_keep", always_keep_);
  always_add_std_    = get_flag("always_add_std", always_add_std_);
  always_init_git_   = get_flag("always_init_git", always_init_git_);
  move_bin_to_wd_    = get_flag("move_bin_to_wd", move_bin_to_wd_);
  clear_before_run_  = get_flag("clear_before_run", clear_before_run_);
  clear_after_run_   = get_flag("clear_after_run", clear_after_run_);
  open_after_init_   = get_flag("open_after_init", open_after_init_);
  open_after_create_ = get_flag("open_after_create_", open_after_create_);

  get_key_with_default(j, "jobs", jobs_, default_jobs_count());

  get_optional_key(j, "token", token_);
  get_optional_key(j, "username", username_);
  get_key_with_default(j, "editor", editor_, std::string(default_editor()));
  get_key_with_default(j, "archiver", archiver_, std::string("ar"));

  if (jsonContainsObject(j, "languages")) {
    for (const auto &[k, v] : j["languages"].items()) {
      auto lang     = from_str<Language>(k).value_or(Language::C);
      auto conf     = v.get<LanguageConf>();
      conf.language = lang;
      languages_.insert_or_assign(lang, conf);
    }
  }
}

void GlobalConfig::write()
{
  json j = {
    { "always_keep", alwaysKeep() },
    { "always_add_std", alwaysAddStd() },
    { "always_init_git", alwaysInitGit() },
    { "move_bin_to_wd", moveBinToWD() },
    { "clear_before_run", clearBeforeRun() },
    { "clear_after_run", clearAfterRun() },
    { "open_after_init", openAfterInit() },
    { "open_after_create", openAfterCreate() },
    { "jobs", jobs_ },
    { "editor", editor_ },
    { "archiver", archiver_ },
  };

  if (!token_.empty())
    j["token"] = token_;
  if (!username_.empty())
    j["username"] = username_;

  j["languages"] = json::object();
  for (const auto &[k, v] : languages_)
    j["languages"][to_str(k).value_or("C")] = v;

  if (fs::write_json(file_, j))
    modified_ = false;
  // FIXME: works on Windows ?
  chmod(file_.c_str(), S_IRUSR | S_IWUSR); // other users are not allowed to see the authentication token
}

GlobalConfig &GlobalConfig::get()
{
  static GlobalConfig instance;
  return instance;
}

void GlobalConfig::setToken(std::string token)
{
  token_    = std::move(token);
  modified_ = true;
}

void GlobalConfig::setUsername(std::string username)
{
  username_ = std::move(username);
  modified_ = true;
}

std::expected<void, Exception> GlobalConfig::set(const std::string_view key, const std::string_view value)
{
  if (key == "always_keep") {
    always_keep_ = to_bool(value);
  } else if (key == "always_add_std") {
    always_add_std_ = to_bool(value);
  } else if (key == "always_init_git") {
    always_init_git_ = to_bool(value);
  } else if (key == "move_bin_to_wd") {
    move_bin_to_wd_ = to_bool(value);
  } else if (key == "clear_before_run") {
    clear_before_run_ = to_bool(value);
  } else if (key == "clear_after_run") {
    clear_after_run_ = to_bool(value);
  } else if (key == "open_after_init") {
    open_after_init_ = to_bool(value);
  } else if (key == "open_after_create") {
    open_after_create_ = to_bool(value);
  } else if (key == "jobs") {
    // TODO: parse integer
  } else if (key == "editor") {
    editor_ = value;
  } else if (key == "archiver") {
    archiver_ = value;
  } else if (key.starts_with("languages.")) {
    // Expected format: languages.<language>.<std|compiler|flags>
    constexpr std::string_view prefix = "languages.";
    const std::string_view     rest   = key.substr(prefix.size());

    const size_t dot = rest.rfind('.');
    if (dot == std::string_view::npos)
      return std::unexpected(
        Exception(ZCError::InvalidValue, "Key '{}' must have the form languages.<language>.<field>", key)
      );

    const auto language = from_str<Language>(rest.substr(0, dot));
    if (!language || *language == Language::Unknown)
      return std::unexpected(
        Exception(ZCError::InvalidValue, "Unknown language '{}' in key '{}'", rest.substr(0, dot), key)
      );

    // Creates the language entry if it doesn't exist yet
    auto &conf =
      languages_
        .try_emplace(
          *language, LanguageConf{ .language = *language, .std = "", .compiler = "", .flags = {} }
        )
        .first->second;

    if (const std::string_view field = rest.substr(dot + 1); field == "std") {
      conf.std = value;
    } else if (field == "compiler") {
      conf.compiler = value;
    } else if (field == "flags") {
      // Space-separated list of flags
      conf.flags.clear();
      for (const auto word : value | std::views::split(' ') |
             std::views::filter([](const auto &w) { return !std::ranges::empty(w); }))
        conf.flags.emplace_back(std::string_view(word));
    } else {
      return std::unexpected(
        Exception(ZCError::InvalidValue, "Unknown language field '{}' in key '{}'", field, key)
      );
    }
  } else {
    return std::unexpected(Exception(ZCError::InvalidValue, "Key '{}' not found", key));
  }
  modified_ = true;
  return {};
}

} // namespace zc::core
