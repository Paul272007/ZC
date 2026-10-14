#pragma once

#include <cassert>

#include "FixedString.hpp"
#include "framework/util/concepts.hpp"

namespace zc::cli
{

template<Assignable T, FixedString LongFlag, char ShortFlag = '\0', bool Required = false>
struct Option {
  T &target;

  std::string_view help;
  std::optional<T> implicit_value{};
  mutable bool     matched = false;

  [[nodiscard]] static constexpr std::string_view longName() { return LongFlag.sv(); }

  [[nodiscard]] static constexpr char shortName() noexcept { return ShortFlag; }

  [[nodiscard]] static constexpr bool hasLongName() noexcept { return !longName().empty(); }

  [[nodiscard]] static constexpr bool hasShortName() noexcept { return ShortFlag != '\0'; }

  [[nodiscard]] static constexpr bool isRequired() noexcept { return Required; }

  [[nodiscard]] bool hasImplicitValue() const noexcept { return implicit_value.has_value(); }

  [[nodiscard]] const T &implicitValue() const
  {
    assert(implicit_value.has_value() && "Option::implicitValue() called without a value");
    return *implicit_value;
  }
};

template<Assignable T, FixedString LongFlag, char ShortFlag = '\0'>
constexpr auto opt(T &target, std::string_view help = "") noexcept
{
  return Option<T, LongFlag, ShortFlag>(target, help);
}

// Gives default value when flag is given without value
template<Assignable T, FixedString LongFlag, char ShortFlag = '\0'>
constexpr auto opt_with_default(T &target, T implicit_val, std::string_view help = "") noexcept
{
  return Option<T, LongFlag, ShortFlag>(target, help, std::make_optional(std::move(implicit_val)));
}

template<Assignable T, FixedString LongFlag, char ShortFlag = '\0'>
constexpr auto required_opt(T &target, std::string_view help = "") noexcept
{
  return Option<T, LongFlag, ShortFlag, true>(target, help);
}

} // namespace zc::cli
