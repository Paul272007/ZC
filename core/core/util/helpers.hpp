#pragma once

#include <string>

#include "core/project/Language.hpp"
#include "framework/util/types.hpp"

namespace zc::core
{

[[nodiscard]] std::string default_compiler(Language language);
[[nodiscard]] std::string default_std(Language language);
[[nodiscard]] std::vector<std::string> default_flags(Language language);
[[nodiscard]] u32 default_jobs_count();
[[nodiscard]] Language linker(const SourceMap &sources);

} // namespace zc::core
