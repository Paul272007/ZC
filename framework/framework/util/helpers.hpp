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

inline bool is_flag(const std::string_view arg)
{
  return arg.starts_with('-');
}

inline bool to_bool(const std::string_view i_bool)
{
  return equals_any(i_bool, "1", "true", "yes", "on");
}

template<Enum EnumType, typename... Args>
EnumType parse_mode(
  std::initializer_list<std::pair<EnumType, bool>> flags, EnumType default_val,
  std::format_string<Args...> fmt, Args &&...args
)
{
  u16      count  = 0;
  EnumType result = default_val;

  for (const auto &pair : flags) {
    if (pair.second) {
      count++;
      result = pair.first;
    }
  }
  if (count > 1)
    throw_or_terminate(Exception(CliError::IncompatibleFlags, fmt, std::forward<Args>(args)...));
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
std::vector<Dest> cast_vector(const std::vector<Src> &src)
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

namespace detail
{

template<typename E, E V>
constexpr std::string_view enum_name_value()
{
  std::string_view s  = __PRETTY_FUNCTION__;
  const auto       eq = s.rfind("V = ");
  if (eq == std::string_view::npos)
    return "";
  s.remove_prefix(eq + 4);
  const auto end = s.find(']');
  if (end != std::string_view::npos)
    s.remove_suffix(s.size() - end);
  if (s.empty() || s.front() == '(')
    return "";
  const auto colons = s.rfind("::");
  if (colons != std::string_view::npos)
    s.remove_prefix(colons + 2);
  return s;
}

template<Enum E, std::size_t... Is>
constexpr std::string_view enum_to_string_impl(E value, std::index_sequence<Is...>)
{
  std::string_view result;
  ([&] {
    if (value == static_cast<E>(Is)) {
      result = enum_name_value<E, static_cast<E>(Is)>();
      return true;
    }
    return false;
  }() ||
   ...);
  return result;
}

template<Enum E, std::size_t... Is>
constexpr std::optional<E> enum_from_string_impl(const std::string_view name, std::index_sequence<Is...>)
{
  std::optional<E> result;
  ([&] {
    constexpr auto val_name = enum_name_value<E, static_cast<E>(Is)>();
    if (!val_name.empty() && val_name.size() == name.size()) {
      bool eq = true;
      for (std::size_t i = 0; i < val_name.size(); ++i) {
        char c1 = val_name[i];
        char c2 = name[i];
        if (c1 >= 'A' && c1 <= 'Z')
          c1 = static_cast<char>(c1 + 32);
        if (c2 >= 'A' && c2 <= 'Z')
          c2 = static_cast<char>(c2 + 32);
        if (c1 != c2) {
          eq = false;
          break;
        }
      }
      if (eq) {
        result = static_cast<E>(Is);
        return true;
      }
    }
    return false;
  }() ||
   ...);
  return result;
}

} // namespace detail

template<Enum E>
constexpr std::expected<std::string_view, Exception> to_str(E value)
{
  std::string_view s = detail::enum_to_string_impl(value, std::make_index_sequence<64>{});
  if (s.empty())
    return std::unexpected(
      Exception(FormatError::InvalidEnum, "Unknown enum value: {}", std::to_underlying(value))
    );
  return s;
}

template<Enum E>
constexpr std::expected<E, Exception> from_str(const std::string_view value)
{
  auto opt = detail::enum_from_string_impl<E>(value, std::make_index_sequence<64>{});
  if (!opt)
    return std::unexpected(Exception(FormatError::InvalidStr, "Unknown enum value: {}", value));
  return *opt;
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
constexpr std::expected<E, Exception> from_str(const std::string_view value)
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
