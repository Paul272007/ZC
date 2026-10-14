#pragma once

#include <concepts>
#include <format>
#include <type_traits>

namespace zc
{

namespace cli
{

struct CommandTag {};

template<typename T>
concept Command = std::derived_from<T, CommandTag>;

} // namespace cli

template<typename T>
concept Assignable = std::assignable_from<T &, T>;

template<typename T>
concept Enum = std::is_enum_v<T>;

template<typename T>
concept Formattable = std::formattable<T, char>;

} // namespace zc
