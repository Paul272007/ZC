#pragma once

#include "FixedString.hpp"
#include "framework/util/concepts.hpp"

namespace zc::cli
{

template<Assignable T, FixedString Name, bool Required = false>
struct Positional {
  T &target;

  std::string_view help;
  mutable bool     matched = false;

  [[nodiscard]] static constexpr std::string_view name() { return Name.sv(); }

  [[nodiscard]] static constexpr bool isRequired() noexcept { return Required; }
};

template<Assignable T, FixedString Name>
constexpr auto pos(T &target, const std::string_view help = "")
{
  return Positional<T, Name>(target, help);
}

template<Assignable T, FixedString Name>
constexpr auto required_pos(T &target, const std::string_view help = "")
{
  return Positional<T, Name, true>(target, help);
}

} // namespace zc::cli
