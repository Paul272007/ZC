#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "helpers.hpp"

namespace zc
{

std::string quote(const std::string &str);

std::string join(const std::vector<std::string> &v, const std::string &separator);

std::string upper(const std::string &text);

std::string lower(const std::string &text);

std::string esc(const std::string &arg);

std::vector<std::string> split(const std::string &str, char delimiter);

constexpr bool iequals(std::string_view a, std::string_view b)
{
  return std::ranges::equal(a, b, [](const char c1, const char c2) {
    return std::tolower(static_cast<unsigned char>(c1)) == std::tolower(static_cast<unsigned char>(c2));
  });
}

constexpr bool is_space(const char c)
{
  return equals_any(c, ' ', '\t', '\n', '\r', '\v');
}

constexpr std::string_view trim(std::string_view sv)
{
  while (!sv.empty() && is_space(sv.front()))
    sv.remove_prefix(1);
  while (!sv.empty() && is_space(sv.back()))
    sv.remove_suffix(1);
  return sv;
}

} // namespace zc
