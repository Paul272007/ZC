#pragma once

#include <map>
#include <string>

#include "../Language.h"
#include "Conf.h"
#include "LanguageConf.h"

namespace zc
{

class GConf : public Conf
{
public:
  bool always_keep              = false;
  bool always_add_std           = false;
  bool open_after_init          = false;
  bool open_after_create        = false;
  bool clear_before_run         = false;
  bool move_bin_to_current_path = false;

  std::string token;
  std::string username;
  std::string editor  = "vim";
  std::string archive = "ar rcs";

  std::map<Language, LanguageConf> languages = {
    { C, { .std = "c17", .compiler = "clang", .flags = { "-Wall", "-Wextra" } } },
    { CXX, { .std = "c++20", .compiler = "clang++", .flags = { "-Wall", "-Wextra" } } },
  };

  [[nodiscard]] static GConf &get();
  GConf(GConf &&)                 = delete;
  GConf &operator=(GConf &&)      = delete;
  GConf(const GConf &)            = delete;
  GConf &operator=(const GConf &) = delete;

  void login(bool force = false);

  void logout();

  ~GConf() override;

protected:
  void load() override;

  void write() override;

private:
  GConf();
};

} // namespace zc
