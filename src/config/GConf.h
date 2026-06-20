/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <string>
#include <vector>

#include "../Language.h"
#include "Conf.h"
#include "LanguageConf.h"

namespace zc
{

class GConf : public Conf
{
public:
  bool always_keep = false;
  bool always_add_std = false;
  bool open_after_init = false;
  bool open_after_create = false;
  bool clear_before_run = false;
  bool move_bin_to_current_path = false;
  std::string token;
  std::string username;
  std::string editor = "vim";
  std::string archive = "ar rcs";
  std::vector<LanguageConf> languages = {
      {C, "c17", "clang", {"-Wall", "-Wextra"}}, {CXX, "c++20", "clang++", {"-Wall", "-Wextra"}}
  };

  static GConf &get();
  GConf(const GConf &) = delete;
  void operator=(const GConf &) = delete;

  void login();

  void logout();

  ~GConf() override;

  LanguageConf get_lang_conf(Language l);

protected:
  void load() override;

  void write() override;

private:
  GConf();
};

} // namespace zc
