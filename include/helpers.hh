#pragma once

#include <filesystem>
#include <string>
#include <vector>

#define ROOT_DIR ".zc"
#define ZC_FILE "zc.json"

/**
 * @brief Returns the root directory of ZC
 */
const std::filesystem::path &getZCRootDir();

/**
 * @brief Finds the root directory of the current ZC project by searching
 * upwards.
 * @return The path to the project root.
 * @throws ZCError if no .zc directory is found in the hierarchy.
 */
const std::filesystem::path &getProjectRoot();

/**
 * @brief Finds the root directory of the ZC project by searching upwards.
 * @param base the base of the search
 * @return The path to the project root.
 * @throws ZCError if no .zc directory is found in the hierarchy.
 */
const std::filesystem::path &getProjectRoot(const std::filesystem::path &base);

std::string escape_shell_arg(const std::string &arg);

std::vector<std::string> split(const std::string &s, char delimiter);

/**
 * @brief Join all vector elements with the separator
 *
 * @param v The vector to be joined
 * @param separator The separator between each element
 */
std::string join(const std::vector<std::string> &v, const std::string &separator);

/**
 * @brief Convert string to uppercase
 *
 * @param s The string to be converted
 */
std::string upper(const std::string &s);