#pragma once

#include <string>

#include "core/project/Language.hpp"
#include "framework/util/types.hpp"

namespace zc::core
{

std::string default_compiler(Language language);
std::string default_std(Language language);
std::vector<std::string> default_flags(Language language);

u32 default_jobs_count();

} // namespace zc::core
