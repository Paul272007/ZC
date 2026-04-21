#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

// clang-format off
#define ROOT_DIR                        ".zc"
#define ZC_FILE                     "zc.json"
#define REGISTRY              "registry.json"
#define EXTERNAL                   "external"
#define INCLUDE_DIR                 "include"
#define LIB_DIR                         "lib"
#define BIN_DIR                         "bin"
#define BUILD_DIR                     "build"
#define TMP_DIR                         "tmp"
#define TEMPLATES                 "templates"
#define PROJECT_TEMPLATES "project_templates"
// clang-format on

/**
 * @brief Returns the root directory of ZC
 */
[[nodiscard]] const std::filesystem::path &getZCRootDir();

/**
 * @brief Finds the root directory of the current ZC project by searching
 * upwards.
 * @return The path to the project root.
 * @throws ZCError if no .zc directory is found in the hierarchy.
 */
[[nodiscard]] const std::filesystem::path getProjectRoot();

/**
 * @brief Finds the root directory of the ZC project by searching upwards.
 * @param base the base of the search
 * @return The path to the project root.
 * @throws ZCError if no .zc directory is found in the hierarchy.
 */
[[nodiscard]] const std::filesystem::path getProjectRoot(const std::filesystem::path &base);

/**
 * @brief Escape arg to use in shell command
 *
 * @param arg The argument to be escaped
 */
[[nodiscard]] std::string escape_shell_arg(const std::string &arg);

[[nodiscard]] std::vector<std::string> split(const std::string &s, char delimiter);

/**
 * @brief Join all vector elements with the separator
 *
 * @param v The vector to be joined
 * @param separator The separator between each element
 */
[[nodiscard]] std::string join(const std::vector<std::string> &v, const std::string &separator);

/**
 * @brief Convert string to uppercase
 *
 * @param s The string to be converted
 */
[[nodiscard]] std::string upper(const std::string &s);

[[nodiscard]] std::string execAndGetOutput(const char *cmd);

[[nodiscard]] std::string urlEncode(const std::string &s);

[[nodiscard]] nlohmann::json parseJsonFile(const std::filesystem::path &file_path);

void writeJsonFile(const nlohmann::json &json, const std::filesystem::path &file_path);

void checkPackageName(const std::string &name);
