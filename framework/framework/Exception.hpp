#pragma once

#include <exception>
#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <utility>

#include "framework/util/concepts.hpp"
#include "framework/util/errors.hpp"
#include "framework/util/types.hpp"

namespace zc
{

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
  #define ZC_EXCEPTIONS_ENABLED
#endif

template<Enum EnumType>
[[nodiscard]] constexpr i32 exit_code(EnumType value) noexcept
{
  return exit_code_base(value) + static_cast<i32>(value);
}

class Exception : public std::exception
{
  i32         code_;
  std::string message_;

public:
  template<Enum EnumType>
  explicit Exception(EnumType value) : code_(exit_code(value)), message_("Error")
  {
  }

  template<Enum EnumType, typename... Args>
  explicit Exception(EnumType value, std::format_string<Args...> fmt, Args &&...args)
    : code_(exit_code(value)), message_("Error: " + std::format(fmt, std::forward<Args>(args)...) + ".")
  {
  }

  [[nodiscard]] i32 code() const noexcept { return code_; }

  [[nodiscard]] const char *what() const noexcept override { return message_.c_str(); }

  template<Enum EnumType>
  [[nodiscard]] bool is(EnumType value) const noexcept
  {
    return code_ == exit_code(value);
  }

  template<Enum EnumType>
  [[nodiscard]] friend bool operator==(const Exception &ex, EnumType value) noexcept
  {
    return ex.is(value);
  }
};

template<typename E>
[[noreturn]] void throw_or_terminate(E &&ex)
{
#ifdef ZC_EXCEPTIONS_ENABLED
  throw std::forward<E>(ex);
#else
  (void)std::fputs(ex.what(), stderr);
  std::terminate();
#endif
}

#define ZC_DO_OR_RETURN(action)                \
  do {                                         \
    if (auto zc_res_ = (action); !zc_res_)     \
      return std::unexpected(zc_res_.error()); \
  } while (false)

#define ZC_DO_OR_THROW(action)             \
  do {                                     \
    if (auto zc_res_ = (action); !zc_res_) \
      throw_or_terminate(zc_res_.error()); \
  } while (false)

} // namespace zc

template<>
struct std::formatter<zc::Exception> : std::formatter<std::string_view> {
  auto format(const zc::Exception &e, std::format_context &ctx) const
  {
    return std::formatter<std::string_view>::format(e.what(), ctx);
  }
};
