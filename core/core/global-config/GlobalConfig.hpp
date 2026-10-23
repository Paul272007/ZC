#pragma once

#include <map>
#include <string>
#include <string_view>

#include "core/Conf.hpp"
#include "core/project/Language.hpp"
#include "core/project/LanguageConf.hpp"

namespace zc::core
{

class GlobalConfig final : public Conf
{
  u32 jobs_count_ = 1;

  std::string archiver_ = "ar";

  std::map<Language, LanguageConf> languages_;

  explicit GlobalConfig();

protected:
  void load() override;
  void write() override;

public:
  GlobalConfig(GlobalConfig &&)                 = delete;
  GlobalConfig(const GlobalConfig &)            = delete;
  GlobalConfig &operator=(GlobalConfig &&)      = delete;
  GlobalConfig &operator=(const GlobalConfig &) = delete;
  ~GlobalConfig() override;

  [[nodiscard]] static GlobalConfig &get();

  // ——— Getters ———

  [[nodiscard]] u32 jobs_count() const noexcept { return jobs_count_; }

  [[nodiscard]] std::string_view archiver() const noexcept { return archiver_; }

  [[nodiscard]] const auto &languages() const noexcept { return languages_; }
};

inline const GlobalConfig &gc()
{
  return GlobalConfig::get();
}

} // namespace zc::core
