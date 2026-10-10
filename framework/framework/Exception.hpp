#pragma once

#include <exception>
#include <format>
#include <string>
#include <string_view>
#include <utility>

#include "framework/util/concepts.hpp"
#include "framework/util/types.hpp"

namespace zc
{

class Exception : public std::exception
{
  i32         code_;
  std::string message_;

public:
  template<Enum EnumType>
  explicit Exception(EnumType value) : code_(static_cast<i32>(value)), message_("Error")
  {
  }

  template<Enum EnumType, typename... Args>
  explicit Exception(EnumType value, std::format_string<Args...> fmt = "", Args &&...args)
    : code_(static_cast<i32>(value)),
      message_("Error: " + std::format(fmt, std::forward<Args>(args)...) + ".")
  {
  }

  [[nodiscard]] i32 code() const noexcept { return code_; }

  [[nodiscard]] const char *what() const noexcept override { return message_.c_str(); }
};

} // namespace zc

template<>
struct std::formatter<zc::Exception> : std::formatter<std::string_view> {
  auto format(const zc::Exception &e, std::format_context &ctx) const
  {
    return std::formatter<std::string_view>::format(e.what(), ctx);
  }
};
