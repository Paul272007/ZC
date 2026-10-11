#pragma once

#include <format>
#include <type_traits>

namespace zc
{

template<typename T>
concept Enum = std::is_enum_v<T>;

template<typename T>
concept Formattable = std::formattable<T, char>;

} // namespace zc
