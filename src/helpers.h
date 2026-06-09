#include <filesystem>

#define ZC_DEV_CONFIG                                                                                        \
  using namespace std;                                                                                       \
  namespace fs = std::filesystem;

// clang-format off

// Global directories and files
#define ZC_DIR             ".zc"
#define CACHE_DIR          "cache"
#define BIN_DIR            "bin"
#define LIB_DIR            "lib"
#define INCLUDE_DIR        "include"
#define TMP_DIR            "tmp"
#define TEMPLATES_DIR      "templates"
#define P_TEMPLATES_DIR    "project_templates"

#define CONFIG_FILE        "config.json"
#define REGISTRY_FILE      "registry.json"

// Project-wide directories and files
#define BUILD_DIR          "build"

#define ZC_FILE            "zc.json"
#define MAKEFILE           "Makefile"

// Distant server files and useful things
#define INDEX_FILE         "index.json"
#define CLIENT_ID          "Ov23liDOGFHUp7VKXTJ7"
#define GH_REPO            "Paul272007/ZC-Registry"

#define DEVICE_CODE_URL    "https://github.com/login/device/code"
#define TOKEN_URL          "https://github.com/login/oauth/access_token"
#define INDEX_URL          "https://paul272007.github.io/ZC-Registry/index.json"

// clang-format on

const std::filesystem::path get_project_root(const std::filesystem::path &base);
const std::filesystem::path get_zc_root();
