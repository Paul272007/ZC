#pragma once

#include <map>
#include <string>

#include "config/Conf.h"
#include "config/Language.h"
#include "config/LanguageConf.h"

namespace zc
{

#define GCONF_BOOL_FIELDS(X)                                                                         \
  X(always_keep, false, "Always keep binaries after program ends ?")                                 \
  X(always_add_std, false, "Always add standard when compiling single files ?")                      \
  X(open_after_init, false, "Always open project in editor after being initialized ?")               \
  X(open_after_create, false, "Always open files in editor after being created ?")                   \
  X(clear_before_run, false, "Always clear terminal before executing programs ?")                    \
  X(move_bin_to_current_path, false, "Always move binary to current path after building packages ?")

#define GCONF_STR_FIELDS(X)                        \
  X(editor, "nvim", "Code editor to use ?")        \
  X(archive, "ar", "Program to create archives ?")

class GConf : public Conf
{
public:
#define DECLARE_BOOL_FIELDS(name, deflt, question) bool name = deflt;
  GCONF_BOOL_FIELDS(DECLARE_BOOL_FIELDS)
#undef DECLARE_BOOL_FIELDS

#define DECLARE_STR_FIELDS(name, deflt, question) std::string name = deflt;
  GCONF_STR_FIELDS(DECLARE_STR_FIELDS)
#undef DECLARE_STR_FIELDS

  std::string token;
  std::string username;

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
  void default_config(bool force = false);
  void set(const std::string &key, const std::string &value);

protected:
  void load() override;

  void write() override;

private:
  GConf();
};

} // namespace zc
