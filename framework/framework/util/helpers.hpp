#pragma once

#include <expected>
#include <format>
#include <unordered_set>
#include <vector>

#include "framework/Exception.hpp"
#include "framework/util/concepts.hpp"
#include "framework/util/errors.hpp"
#include "framework/util/types.hpp"

namespace zc
{

template<typename T, typename... Args>
constexpr bool equals_any(const T &value, const Args... args)
{
  return ((value == args) || ...);
}

template<Enum EnumType, typename... Args>
EnumType parse_mode(
  std::initializer_list<std::pair<EnumType, bool>> flags, EnumType dflt, std::format_string<Args...> fmt,
  Args &&...args
)
{
  u16      count  = 0;
  EnumType result = dflt;

  for (const auto &pair : flags) {
    if (pair.second) {
      count++;
      result = pair.first;
    }
  }
  if (count > 1)
    throw Exception(CliError::IncompatibleFlags, fmt, std::forward<Args>(args)...);
  return result;
}

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

template<typename Src, typename Dest>
void cast_vector(const std::vector<Src> &src)
{
  std::vector<Dest> dest;
  dest.reserve(src.size());
  for (const auto &item : src)
    dest.emplace_back(item);
  return dest;
}

template<typename T, std::size_t N, typename F>
consteval auto make_static_array(F fn)
{
  auto             vec = fn();
  std::array<T, N> arr{};
  for (std::size_t i = 0; i < N; i++)
    arr[i] = vec[i];
  return arr;
}

#ifdef __clang__

template<Enum E>
constexpr std::expected<std::string_view, Exception> to_str([[maybe_unused]] E value)
{
  return "";
}

template<Enum E>
constexpr std::expected<E, Exception> from_str([[maybe_unused]] const std::string &value)
{
  return static_cast<E>(0);
}

#else

  #include <meta>

template<Enum E>
constexpr std::expected<std::string_view, Exception> to_str(E value)
{
  constexpr auto count = std::meta::enumerators_of(^^E).size();

  static constexpr auto enumerators =
    make_static_array<std::meta::info, count>([] { return std::meta::enumerators_of(^^E); });

  template for (constexpr auto e : enumerators)
  {
    if (value == [:e:])
      return std::meta::identifier_of(e);
  }
  return std::unexpected(
    Exception(FormatError::InvalidEnum, "Unknown enum value: {}", std::to_underlying(value))
  );
}

template<Enum E>
constexpr std::expected<E, Exception> from_str(const std::string &value)
{
  constexpr auto count = std::meta::enumerators_of(^^E).size();

  static constexpr auto enumerators =
    make_static_array<std::meta::info, count>([] { return std::meta::enumerators_of(^^E); });

  template for (constexpr auto e : enumerators)
  {
    if (iequals(value, std::meta::identifier_of(e)))
      return [:e:];
  }
  return std::unexpected(Exception(FormatError::InvalidStr, "Unknow enum value: {}", value));
}

#endif

} // namespace zc
