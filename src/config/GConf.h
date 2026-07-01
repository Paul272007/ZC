#pragma once

#include <map>
#include <string>

#include "config/Conf.h"
#include "config/Language.h"
#include "config/LanguageConf.h"

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
  std::string editor;
  std::string archive;

  std::map<Language, LanguageConf> languages = {
    { C, { .std = "c17", .compiler = "clang", .flags = { "-Wall", "-Wextra" } } },
    { CXX, { .std = "c++20", .compiler = "clang++", .flags = { "-Wall", "-Wextra" } } },
  };

  [[nodiscard]] static GConf &get();
  GConf(GConf &&)                 = delete;
  GConf &operator=(GConf &&)      = delete;
  GConf(const GConf &)            = delete;
  GConf &operator=(const GConf &) = delete;
  ~GConf() override;

  void login(bool force = false);
  void logout();
  void edit_config(bool force = false);

protected:
  void load() override;

  void write() override;

private:
  GConf();
};

} // namespace zc
