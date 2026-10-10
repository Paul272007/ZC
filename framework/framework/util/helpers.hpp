#pragma once

#include <unordered_set>
#include <vector>

template<typename T>
void merge(const std::vector<T> &src, std::vector<T> &dest)
{
  std::unordered_set existing(dest.begin(), dest.end());

  for (const auto &item : src) {
    if (!existing.contains(item)) {
      dest.push_back(item);
      existing.insert(item);
    }
  }
}

template<typename T, typename... Args>
constexpr bool equals_any(const T &value, const Args... args)
{
  return ((value == args) || ...);
}
