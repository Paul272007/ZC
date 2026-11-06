#pragma once

#include <string>
#include <string_view>

#include "core/project/Language.hpp"
#include "framework/util/types.hpp"

namespace zc::core
{

[[nodiscard]] std::string_view default_editor();
[[nodiscard]] std::string_view default_compiler(Language language);
[[nodiscard]] std::string_view default_std(Language language);

/// @brief Get default compiling flags for compiler
[[nodiscard]] std::vector<std::string> default_flags(Language language);

/// @brieg Get default number of files to be compiled simultaneously
[[nodiscard]] u32 default_jobs_count();

/// @brief Get required linker based on the languages found in the given sources
[[nodiscard]] Language linker(const SourceMap &sources);

} // namespace zc::core
