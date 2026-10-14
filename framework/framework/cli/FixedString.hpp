#pragma once
#include "framework/util/types.hpp"

namespace zc::cli
{

template<u64 N>
struct FixedString {
  char buffer[N]{};

  consteval FixedString(const char (&str)[N])
  {
    for (u64 i = 0; i < N; i++)
      buffer[i] = str[i];
  }

  [[nodiscard]] constexpr std::string_view sv() const { return { buffer, N - 1 }; }

  [[nodiscard]] constexpr bool operator==(const FixedString &) const = default;

  template<u64 M>
  [[nodiscard]] constexpr bool operator==(const FixedString<M> &other) const
  {
    return sv() == other.sv();
  }
};

template<u64 N>
FixedString(const char (&)[N]) -> FixedString<N>;

} // namespace zc::cli
