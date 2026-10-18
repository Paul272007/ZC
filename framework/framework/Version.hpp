#pragma once

#include <format>
#include <nlohmann/json.hpp>
#include <sstream>
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

public:
  constexpr Version(const std::string &text)
  {
    if (text.empty()) {
      (*this) = empty();
      return;
    }
    if (text == "latest") {
      (*this) = latest();
      return;
    }

    std::stringstream ss(text);
    std::vector<i32>  parts;

    try {
      std::string segment;
      while (std::getline(ss, segment, '.')) // Get 3 first numbers in string
        if (!segment.empty())
          parts.push_back(std::stoi(segment));
    } catch (const std::exception &) {
      throw Exception(FormatError::InvalidVersion, "Invalid version format: {}", text);
    }

    if (!parts.empty())
      major_ = parts.at(0);
    if (parts.size() >= 2)
      minor_ = parts.at(1);
    if (parts.size() >= 3)
      patch_ = parts.at(2);
  }

  constexpr Version(const i32 major = 0, const i32 minor = 0, const i32 patch = 0) noexcept
    : major_(major), minor_(minor), patch_(patch)
  {
  }

  friend std::ostream &operator<<(std::ostream &s, const Version &v) { return s << v.string(); }

  [[nodiscard]] auto operator<=>(const Version &) const = default;

  [[nodiscard]] std::string string() const { return std::format("{}.{}.{}", major_, minor_, patch_); }

  [[nodiscard]] static Version empty() { return { 0, 0, 0 }; }

  [[nodiscard]] static Version latest() { return { -1, -1, -1 }; }

  [[nodiscard]] bool isEmpty() const { return *this == empty(); }

  [[nodiscard]] bool isLatest() const { return *this == latest(); }

  [[nodiscard]] i32 major() const { return major_; }

  [[nodiscard]] i32 minor() const { return minor_; }

  [[nodiscard]] i32 patch() const { return patch_; }
};

inline void from_json(const json &j, Version &v)
{
  v = j.get<std::string>();
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
