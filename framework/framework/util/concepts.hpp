#pragma once

#include <type_traits>

namespace zc
{

template<typename T>
concept Enum = std::is_enum_v<T>;

} // namespace zc
