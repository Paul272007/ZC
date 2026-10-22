#pragma once

#include <format>
#include <string>

namespace zc::core
{

#ifdef ZC_WINDOWS
inline constexpr auto user_home_env = "USERPROFILE";
#else
inline constexpr auto user_home_env = "HOME";
#endif

} // namespace zc::core
