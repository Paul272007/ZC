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

inline std::string static_lib_fmt(const char *name)
{
#if defined(_WIN32) || defined(_WIN64)
  return std::format("{}.a", name);
#else
  return std::format("lib{}.a", name);
#endif
}

inline std::string shared_lib_fmt(const char *name)
{
#ifdef ZC_WINDOWS
  return std::format("{}.dll", name);
#elifdef ZC_MACOS
  return std::format("lib{}.dylib", name);
#else
  return std::format("lib{}.so", name);
#endif
}

} // namespace zc::core
