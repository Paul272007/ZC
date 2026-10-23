#include "core/global-config/GlobalConfig.hpp"

#include <string>

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
  if (const auto res = fs::read_json(file_)) {
    const auto &j = *res;
    get_key_with_default(j, "jobs", jobs_count_, default_jobs_count());
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
}

void GlobalConfig::write()
{
  json j = {
    { "jobs", jobs_count_ },
    { "archiver", archiver_ },
  };

  j["languages"] = json::object();
  for (const auto &[k, v] : languages_)
    j["languages"][to_str(k).value_or("C")] = v;

  if (fs::write_json(file_, j))
    modified_ = false;
}

GlobalConfig &GlobalConfig::get()
{
  static GlobalConfig instance;
  return instance;
}

} // namespace zc::core
