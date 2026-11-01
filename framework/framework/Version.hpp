#pragma once

#include <array>
#include <charconv>
#include <concepts>
#include <format>
#include <nlohmann/json.hpp>
#include <ostream>
#include <ranges>
#include <string>
#include <string_view>

#include "Exception.hpp"
#include "framework/util/errors.hpp"
#include "framework/util/types.hpp"

namespace zc
{

class Version
{
  i32 major_ = 0;
  i32 minor_ = 0;
  i32 patch_ = 0;

  constexpr void parse(const std::string_view text)
  {
    if (text.empty())
      return;

    if (text == "latest") {
      *this = latest();
      return;
    }

    std::array<i32, 3> parts{};
    u8                 idx = 0;

    for (const auto part : text | std::views::split('.')) {
      const std::string_view seg(part);
      if (seg.empty())
        continue;

      i32         val = 0;
      const char *end = seg.data() + seg.size();
      auto [ptr, ec]  = std::from_chars(seg.data(), end, val);
      if (ec != std::errc{} || ptr != end)
        throw Exception(FormatError::InvalidVersion, "Invalid version format: {}", text);

      if (idx < parts.size())
        parts[idx++] = val;
    }

    major_ = parts[0];
    minor_ = parts[1];
    patch_ = parts[2];
  }

public:
  template<std::convertible_to<std::string_view> S>
  constexpr Version(const S &text)
  {
    parse(text);
  }

  constexpr Version(const i32 major = 0, const i32 minor = 0, const i32 patch = 0) noexcept
    : major_(major), minor_(minor), patch_(patch)
  {
  }

  friend std::ostream &operator<<(std::ostream &s, const Version &v) { return s << v.string(); }

  [[nodiscard]] constexpr auto operator<=>(const Version &) const = default;

  [[nodiscard]] std::string string() const
  {
    if (isEmpty())
      return "default";
    if (isLatest())
      return "latest";
    return std::format("{}.{}.{}", major_, minor_, patch_);
  }

  [[nodiscard]] static constexpr Version empty() noexcept { return { 0, 0, 0 }; }

  [[nodiscard]] static constexpr Version latest() noexcept { return { -1, -1, -1 }; }

  [[nodiscard]] constexpr bool isEmpty() const noexcept { return *this == empty(); }

  [[nodiscard]] constexpr bool isLatest() const noexcept { return *this == latest(); }

  [[nodiscard]] constexpr i32 major() const noexcept { return major_; }

  [[nodiscard]] constexpr i32 minor() const noexcept { return minor_; }

  [[nodiscard]] constexpr i32 patch() const noexcept { return patch_; }
};

inline void from_json(const json &j, Version &v)
{
  v = j.get<std::string_view>();
}

inline void to_json(json &j, const Version &v)
{
  j = v.string();
}

} // namespace zc

template<>
struct std::formatter<zc::Version> : std::formatter<std::string_view> {
  template<typename FormatContext>
  auto format(const zc::Version &v, FormatContext &ctx) const
  {
    return std::formatter<std::string_view>::format(v.string(), ctx);
  }
};
