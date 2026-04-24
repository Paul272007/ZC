#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

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

void checkPackageName(const std::string &name);
