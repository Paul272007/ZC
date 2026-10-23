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
  bool always_keep_       : 1 = false;
  bool always_add_std_    : 1 = false;
  bool move_bin_to_wd_    : 1 = false;
  bool clear_before_run_  : 1 = false;
  bool clear_after_run_   : 1 = false;
  bool open_after_init_   : 1 = false;
  bool open_after_create_ : 1 = false;

  u32 jobs_ = 1;

  std::string token_;
  std::string username_;
  std::string editor_   = "nvim";
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

  [[nodiscard]] bool alwaysKeep() const noexcept { return always_keep_; }

  [[nodiscard]] bool alwaysAddStd() const noexcept { return always_add_std_; }

  [[nodiscard]] bool moveBinToWD() const noexcept { return move_bin_to_wd_; }

  [[nodiscard]] bool clearBeforeRun() const noexcept { return clear_before_run_; }

  [[nodiscard]] bool clearAfterRun() const noexcept { return clear_after_run_; }

  [[nodiscard]] bool openAfterInit() const noexcept { return open_after_init_; }

  [[nodiscard]] bool openAfterCreate() const noexcept { return open_after_create_; }

  [[nodiscard]] u32 jobs() const noexcept { return jobs_; }

  [[nodiscard]] std::string_view archiver() const noexcept { return archiver_; }

  [[nodiscard]] const auto &languages() const noexcept { return languages_; }
};

inline const GlobalConfig &gc()
{
  return GlobalConfig::get();
}

} // namespace zc::core
