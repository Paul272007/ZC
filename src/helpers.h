#pragma once

#include <filesystem>
#include <initializer_list>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <utility>
#include <vector>

#include "excepts/ZCException.h"

#define ZC_DEV_CONFIG                                                                                        \
  using namespace std;                                                                                       \
  namespace fs = std::filesystem;

#define ZC_DEV_CONFIG_JSON ZC_DEV_CONFIG using json = nlohmann::json;

// clang-format off
#if defined(_WIN32) || defined(_WIN64)
#define BIN_NAME        (pconf.target + ".exe")
#define STATIC_LIB_NAME (pconf.target + ".a")
#define SHARED_LIB_NAME (pconf.target + ".lib")
#define RM_COMMAND      "del"
#define MKDIR_COMMAND   "mkdir"
#define USER_HOME_ENV   "USERPROFILE"
#define HIDE_OUTPUT     " > NUL 2>&1" // Powershell : " *> $null"
#elifdef __APPLE__
#define BIN_NAME        pconf.target
#define STATIC_LIB_NAME ("lib" + pconf.target + ".a")
#define SHARED_LIB_NAME ("lib" + pconf.target + ".dylib")
#define RM_COMMAND      "rm"
#define MKDIR_COMMAND   "mkdir -p"
#define USER_HOME_ENV   "HOME"
#define HIDE_OUTPUT     " &>/dev/null"
#else
#define BIN_NAME        pconf.target
#define STATIC_LIB_NAME ("lib" + pconf.target + ".a")
#define SHARED_LIB_NAME ("lib" + pconf.target + ".so")
#define RM_COMMAND      "rm"
#define MKDIR_COMMAND   "mkdir -p"
#define USER_HOME_ENV   "HOME"
#define HIDE_OUTPUT     " &>/dev/null"
#endif

#define FORBIDDEN_NAMES    {"Makefile", "CMakeLists.txt", "zc.json", "registry.json", "index.json", "build", ".cache", "cache"}
#define FORBIDDEN_CHARS    {'@', '#', ' ', '*', '!', '?', '{', '}', '[', ']', '(', ')', '"', '\'', '/', '\\', '|', '~', '&', ';', ':', '$'}

// Global directories and files
#define ZC_DIR             ".zc"
#define ZC_CACHE_DIR       "cache"
#define BIN_DIR            "bin"
#define LIB_DIR            "lib"
#define INCLUDE_DIR        "include"
#define TMP_DIR            "tmp"
#define TEMPLATES_DIR      "templates"
#define P_TEMPLATES_DIR    "project_templates"

#define CONFIG_FILE        "config.json"
#define REGISTRY_FILE      "registry.json"

// Project-wide directories and files
#define SRC_DIR            "src"
#define BUILD_DIR          "build"
#define PROJECT_CACHE_DIR  ".cache"

#define ZC_FILE            "zc.json"
#define MAKEFILE           "Makefile"

// Distant server files and useful things
#define INDEX_FILE         "index.json"
#define CLIENT_ID          "Ov23liDOGFHUp7VKXTJ7"
#define GH_REPO            "Paul272007/ZC-Registry"

#define DEVICE_CODE_URL    "https://github.com/login/device/code"
#define TOKEN_URL          "https://github.com/login/oauth/access_token"
#define INDEX_URL          "https://paul272007.github.io/ZC-Registry/index.json"

namespace zc
{

struct Target {
  const std::string name;
  const std::string version;
};

using Targets = std::vector<Target>;

// clang-format on

std::filesystem::path get_project_root(const std::filesystem::path &base = std::filesystem::current_path());
const std::filesystem::path &get_zc_root();
void create_zc_root();
Targets parse_targets(const std::vector<std::string> &targets);
void check_name(const std::string &name);

void extract(const std::filesystem::path &archive, const std::filesystem::path &dest);
std::string sha256(const std::filesystem::path &path);
std::string base64_encode(const std::string &in);

std::string upper(const std::string &text);
std::string lower(const std::string &text);
std::string escape_shell_arg(const std::string &arg);
std::string exec_command(const std::string &cmd);

template <typename EnumType>
EnumType parse_mode(
    std::initializer_list<std::pair<EnumType, bool>> flags, EnumType default_mode,
    const std::string &error_msg = "Incompatible flags: multiple modes selected."
)
{
  int count = 0;
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

template <typename T> void get_key(const nlohmann::json &json_conf, const std::string &key, T &variable)
{
  if (!json_conf.contains(key))
    throw ZCException(ZCE_MISSING_PROPERTY, "Expected property '" + key + "' missing.");

  try
  {
    json_conf.at(key).get_to(variable);
  }
  catch ([[maybe_unused]] const nlohmann::json::type_error &_)
  {
    throw ZCException(ZCE_TYPE_ERROR, "Configuration error: key '" + key + "' has the wrong type");
  }
}

template <typename T>
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
  catch ([[maybe_unused]] const nlohmann::json::type_error &_)
  {
    throw ZCException(ZCE_TYPE_ERROR, "Configuration error: key '" + key + "' has the wrong type");
  }
}

} // namespace zc
