#pragma once

#include <string_view>

#include "framework/fs.hpp"

namespace zc::core
{

// Global directories and files
#ifdef ZC_DEBUG
inline constexpr std::string_view zc_root_dir = ".zc-debug";
#else
inline constexpr std::string_view zc_root_dir = ".zc";
#endif
inline constexpr std::string_view zc_cache_dir       = "cache";
inline constexpr std::string_view zc_bin_dir         = "bin";
inline constexpr std::string_view zc_lib_dir         = "lib";
inline constexpr std::string_view zc_include_dir     = "include";
inline constexpr std::string_view zc_tmp_dir         = "tmp";
inline constexpr std::string_view zc_templates_dir   = "templates";
inline constexpr std::string_view zc_p_templates_dir = "project-templates";

inline constexpr std::string_view zc_config_file         = "config.json";
inline constexpr std::string_view zc_registry_file       = "registry.json";
inline constexpr std::string_view zc_ninja_template_file = "build.ninja.template";

// Project directories and files
inline constexpr std::string_view zc_src_dir           = "src";
inline constexpr std::string_view zc_build_dir         = "zc-build";
inline constexpr std::string_view zc_project_cache_dir = ".cache";

inline constexpr std::string_view zc_project_file    = "zc.json";
inline constexpr std::string_view zc_target_file     = "target.json";
inline constexpr std::string_view zc_ninja_file      = "build.ninja";
inline constexpr std::string_view zc_build_mode_file = ".zc-build-mode";

// Server directories, files and other useful things
inline constexpr std::string_view zc_index_file = "index.json";
inline constexpr std::string_view zc_client_id  = "Ov23liDOGFHUp7VKXTJ7";
inline constexpr std::string_view zc_gh_repo    = "Paul272007/ZC-Registry";

inline constexpr std::string_view zc_device_code_url = "https://github.com/login/device/code";
inline constexpr std::string_view zc_token_url       = "https://github.com/login/oauth/access_token";
inline constexpr std::string_view zc_index_url = "https://paul272007.github.io/ZC-Registry/index.json";

// Global directories
[[nodiscard]] const stdfs::path &zc_dir();
[[nodiscard]] const stdfs::path &tmp_dir();
[[nodiscard]] const stdfs::path &cache_dir();
[[nodiscard]] const stdfs::path &templates_dir();
[[nodiscard]] const stdfs::path &bin_links_dir();
[[nodiscard]] const stdfs::path &lib_links_dir();
[[nodiscard]] const stdfs::path &include_links_dir();

// Project directories
[[nodiscard]] std::expected<stdfs::path, Exception>
project_root_dir(const stdfs::path &path = stdfs::current_path());
[[nodiscard]] bool in_a_project(const stdfs::path &path = stdfs::current_path());

} // namespace zc::core
