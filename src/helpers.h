#pragma once

#include <cstddef>
#include <filesystem>
#include <initializer_list>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <utility>
#include <vector>

#include "excepts/ZCException.h"

#define ZC_DEV_CONFIG             \
  using namespace std;            \
  namespace fs = std::filesystem;

#define ZC_DEV_CONFIG_JSON ZC_DEV_CONFIG using json = nlohmann::json;

#if defined(_WIN32) || defined(_WIN64)
  #define BIN_NAME(name)        (name + ".exe")
  #define STATIC_LIB_NAME(name) (name + ".a")
  #define SHARED_LIB_NAME(name) (name + ".dll")
  #define RM_COMMAND            "del"
  #define MKDIR_COMMAND         "mkdir"
  #define USER_HOME_ENV         "USERPROFILE"
// TODO: add other HIDE_... macros
  #define HIDE_ALL              " > NUL 2>&1" // Powershell : " *> $null"
#elifdef __APPLE__
  #define BIN_NAME(name)        name
  #define STATIC_LIB_NAME(name) ("lib" + name + ".a")
  #define SHARED_LIB_NAME(name) ("lib" + name + ".dylib")
  #define RM_COMMAND            "rm"
  #define MKDIR_COMMAND         "mkdir -p"
  #define USER_HOME_ENV         "HOME"
  #define HIDE_OUT              " 1>/dev/null"
  #define HIDE_ERR              " 2>/dev/null"
  #define HIDE_ALL              " &>/dev/null"
  #define ERR_TO_OUT            " 2>&1"
  #define OUT_TO_ERR            " 1>&2"
#else
  #define BIN_NAME(name)        name
  #define STATIC_LIB_NAME(name) ("lib" + name + ".a")
  #define SHARED_LIB_NAME(name) ("lib" + name + ".so")
  #define RM_COMMAND            "rm"
  #define MKDIR_COMMAND         "mkdir -p"
  #define USER_HOME_ENV         "HOME"
  #define HIDE_OUT              " 1>/dev/null"
  #define HIDE_ERR              " 2>/dev/null"
  #define HIDE_ALL              " &>/dev/null"
  #define ERR_TO_OUT            " 2>&1"
  #define OUT_TO_ERR            " 1>&2"
#endif

#define CAA  const auto &
#define elif else if
#define FORBIDDEN_NAMES                                                                                  \
  { "Makefile", "CMakeLists.txt", "zc.json", "registry.json", "index.json", "build", ".cache", "cache" }
#define FORBIDDEN_CHARS                                       \
  { '@', '#', ' ',  '*', '!',  '?', '{', '}', '[', ']', '(',  \
    ')', '"', '\'', '/', '\\', '|', '~', '&', ';', ':', '$' }

// Global directories and files
#define ZC_DIR            ".zc"
#define ZC_CACHE_DIR      "cache"
#define BIN_DIR           "bin"
#define LIB_DIR           "lib"
#define INCLUDE_DIR       "include"
#define TMP_DIR           "tmp"
#define TEMPLATES_DIR     "templates"
#define P_TEMPLATES_DIR   "project_templates"

#define CONFIG_FILE       "config.json"
#define REGISTRY_FILE     "registry.json"

// Project-wide directories and files
#define SRC_DIR           "src"
#define BUILD_DIR         "build"
#define PROJECT_CACHE_DIR ".cache"

#define ZC_FILE           "zc.json"
#define MAKEFILE          "Makefile"
#define BUILD_MODE_FILE   ".zc_build_mode"

// Distant server files and useful things
#define INDEX_FILE        "index.json"
#define CLIENT_ID         "Ov23liDOGFHUp7VKXTJ7"
#define GH_REPO           "Paul272007/ZC-Registry"

#define DEVICE_CODE_URL   "https://github.com/login/device/code"
#define TOKEN_URL         "https://github.com/login/oauth/access_token"
#define INDEX_URL         "https://paul272007.github.io/ZC-Registry/index.json"

namespace zc
{

class Version;
class Interface;
class Network;
class TemplateEngine;
class GConf;
class Registry;

using Target = std::pair<std::string, Version>;

zc::Interface &ui();
zc::Network &net();
zc::TemplateEngine &te();
zc::GConf &gc();
zc::Registry &rg();

// Operations on ZC directories
const std::filesystem::path &zc_root();
std::filesystem::path get_project_root(const std::filesystem::path &base = std::filesystem::current_path());
bool is_in_a_zc_project();
void create_zc_root();

// Operations on files
std::string read_file(const std::filesystem::path &file);
void write_file(const std::filesystem::path &file, const std::string &content);
nlohmann::json read_json(const std::filesystem::path &file_path);
void write_json(const nlohmann::json &json, const std::filesystem::path &file_path);

// Misc
void check_name(const std::string &name);
void extract(const std::filesystem::path &archive, const std::filesystem::path &dest);
bool has_pkg_config();
std::string get_pkg_config_flags(const std::string &pkg_name, bool cflags);
std::string exec_command(const std::string &cmd);
std::string sha256(const std::filesystem::path &path);
std::string base64_encode(const std::string &in);
size_t get_jobs_count(int input_jobs);
std::vector<Target> parse_targets(const std::vector<std::string> &targets);

// String utilities
std::string pretty_path(const std::filesystem::path &path);
std::string join(const std::vector<std::string> &v, const std::string &separator = " ");
std::string upper(const std::string &text);
std::string lower(const std::string &text);
std::string esc(const std::string &arg);

inline std::string stringify(const std::string &text)
{
  return "\"" + text + "\"";
}

// Vector utilities
std::vector<std::filesystem::path> str_to_path(const std::vector<std::string> &vec);
std::vector<std::string> split(const std::string &str, char delimiter = ' ');
void merge(const std::vector<std::string> &src, std::vector<std::string> &dest);

template<typename EnumType>
EnumType parse_mode(
  std::initializer_list<std::pair<EnumType, bool>> flags, EnumType default_mode,
  const std::string &error_msg = "Incompatible flags: multiple modes selected."
)
{
  int      count  = 0;
  EnumType result = default_mode;

  for (const auto &pair : flags)
  {
    if (pair.second)
    {
      count++;
      result = pair.first;
    }
  }
  if (count > 1)
    throw ZCException(ZCE_INCOMPATIBLE_FLAGS, error_msg);
  return result;
}

template<typename T>
void get_key(const nlohmann::json &json_conf, const std::string &key, T &variable)
{
  if (!json_conf.contains(key))
    throw ZCException(ZCE_MISSING_PROPERTY, "Expected key '" + key + "' missing.");

  try
  {
    json_conf.at(key).get_to(variable);
  }
  catch (const nlohmann::json::type_error &)
  {
    throw ZCException(ZCE_TYPE_ERROR, "Configuration error: key '" + key + "' has the wrong type");
  }
}

template<typename T>
void get_key(const nlohmann::json &json_conf, const std::string &key, T &variable, T default_value)
{
  if (!json_conf.contains(key))
  {
    variable = default_value;
    return;
  }
  try
  {
    json_conf.at(key).get_to(variable);
  }
  catch (const nlohmann::json::type_error &)
  {
    throw ZCException(ZCE_TYPE_ERROR, "Configuration error: key '" + key + "' has the wrong type");
  }
}

} // namespace zc
