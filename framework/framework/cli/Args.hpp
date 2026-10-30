#pragma once

#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "framework/cli/FixedString.hpp"
#include "framework/util/concepts.hpp"

namespace zc::cli
{

namespace detail
{

struct FlagTag {};

struct PositionalTag {};

template<typename T>
class Arg
{
protected:
  T                value_{};
  std::string_view desc_;
  bool             matched_ = false;

  constexpr explicit Arg(const std::string_view desc = "") : desc_(desc) {}

  constexpr Arg(T default_value, const std::string_view desc)
    : value_(std::move(default_value)), desc_(desc)
  {
  }

public:
  using value_type = T;

  /// @brief Get description
  [[nodiscard]] constexpr std::string_view description() const noexcept { return desc_; }

  /// @brief Whether this argument was given on the command line
  [[nodiscard]] constexpr bool isSet() const noexcept { return matched_; }

  /// @brief Get underlying value
  [[nodiscard]] constexpr const T &operator*() const noexcept { return value_; }

  [[nodiscard]] constexpr T &operator*() noexcept { return value_; }

  [[nodiscard]] constexpr const T *operator->() const noexcept requires std::is_class_v<T>
  {
    return &value_;
  }

  [[nodiscard]] constexpr T *operator->() noexcept requires std::is_class_v<T> { return &value_; }

  /// @brief Raw storage access for the CLI parser
  [[nodiscard]] constexpr T &storage() noexcept { return value_; }

  [[nodiscard]] constexpr const T &storage() const noexcept { return value_; }

  constexpr void setMatched(const bool m = true) noexcept { matched_ = m; }
};

template<typename T>
class ListArg : public Arg<std::vector<T>>
{
protected:
  using Arg<std::vector<T>>::Arg;

public:
  using elem_type = T;

  [[nodiscard]] constexpr auto begin() const noexcept { return this->value_.begin(); }

  [[nodiscard]] constexpr auto begin() noexcept { return this->value_.begin(); }

  [[nodiscard]] constexpr auto end() const noexcept { return this->value_.end(); }

  [[nodiscard]] constexpr auto end() noexcept { return this->value_.end(); }
};

template<FixedString LongName, char ShortName>
struct FlagNames : FlagTag {
  static_assert(!LongName.sv().empty(), "Long flag name cannot be empty");
  static_assert(
    !LongName.sv().starts_with('-'), "Do not prefix flag with dashes ('--' is automatically added)"
  );
  static_assert(
    LongName.sv() != "help" && LongName.sv() != "version",
    "'--help' and '--version' are reserved flag names"
  );
  static_assert(ShortName != '-' && ShortName != '=', "Invalid short flag character");
  static_assert(ShortName != 'h' && ShortName != 'V', "'-h' and '-V' are reserved short flags");

  [[nodiscard]] static constexpr std::string_view longName() noexcept { return LongName.sv(); }

  [[nodiscard]] static constexpr char shortName() noexcept { return ShortName; }

  [[nodiscard]] static constexpr bool hasShortName() noexcept { return ShortName != '\0'; }
};

template<FixedString Name>
struct PositionalName : PositionalTag {
  static_assert(!Name.sv().empty(), "Positional argument name cannot be empty");
  static_assert(!Name.sv().starts_with('-'), "Positional argument name cannot start with '-'");

  [[nodiscard]] static constexpr std::string_view name() noexcept { return Name.sv(); }
};

template<u64 N>
consteval FixedString<N> snake_to_kebab(const char (&str)[N])
{
  FixedString<N> res(str);
  for (u64 i = 0; i < N - 1; ++i)
    if (res.buffer[i] == '_')
      res.buffer[i] = '-';
  return res;
}

} // namespace detail

// ——— Flags / Options ———

template<FixedString LongName, char ShortName = '\0'>
class Flag : public detail::Arg<bool>, public detail::FlagNames<LongName, ShortName>
{
public:
  constexpr explicit Flag(const std::string_view desc = "") noexcept : detail::Arg<bool>(desc) {}

  [[nodiscard]] static constexpr bool isRequired() noexcept { return false; }

  [[nodiscard]] static constexpr bool isSwitch() noexcept { return true; }

  [[nodiscard]] static constexpr bool isList() noexcept { return false; }

  [[nodiscard]] constexpr operator bool() const noexcept { return this->value_; }
};

template<Assignable T, FixedString LongName, char ShortName = '\0'>
class Option : public detail::Arg<T>, public detail::FlagNames<LongName, ShortName>
{
  static_assert(!std::is_same_v<T, bool>, "Use zc::cli::Flag for boolean flags");

public:
  constexpr Option(T default_value, const std::string_view desc = "")
    : detail::Arg<T>(std::move(default_value), desc)
  {
  }

  [[nodiscard]] static constexpr bool isRequired() noexcept { return false; }

  [[nodiscard]] static constexpr bool isSwitch() noexcept { return false; }

  [[nodiscard]] static constexpr bool isList() noexcept { return false; }

  [[nodiscard]] constexpr explicit operator bool() const noexcept { return this->isSet(); }
};

template<Assignable T, FixedString LongName, char ShortName = '\0'>
class RequiredOption : public detail::Arg<T>, public detail::FlagNames<LongName, ShortName>
{
  static_assert(!std::is_same_v<T, bool>, "Use zc::cli::Flag for boolean flags");

public:
  constexpr explicit RequiredOption(const std::string_view desc = "") : detail::Arg<T>(desc) {}

  [[nodiscard]] static constexpr bool isRequired() noexcept { return true; }

  [[nodiscard]] static constexpr bool isSwitch() noexcept { return false; }

  [[nodiscard]] static constexpr bool isList() noexcept { return false; }
};

template<typename T, FixedString LongName, char ShortName = '\0'>
class ListOption : public detail::ListArg<T>, public detail::FlagNames<LongName, ShortName>
{
public:
  constexpr explicit ListOption(const std::string_view desc = "") : detail::ListArg<T>(desc) {}

  [[nodiscard]] static constexpr bool isRequired() noexcept { return false; }

  [[nodiscard]] static constexpr bool isSwitch() noexcept { return false; }

  [[nodiscard]] static constexpr bool isList() noexcept { return true; }

  [[nodiscard]] static constexpr bool allowsEmpty() noexcept { return true; }

  [[nodiscard]] constexpr explicit operator bool() const noexcept { return this->isSet(); }
};

template<typename T, FixedString LongName, char ShortName = '\0'>
class RequiredListOption : public detail::ListArg<T>, public detail::FlagNames<LongName, ShortName>
{
public:
  constexpr explicit RequiredListOption(const std::string_view desc = "") : detail::ListArg<T>(desc) {}

  [[nodiscard]] static constexpr bool isRequired() noexcept { return true; }

  [[nodiscard]] static constexpr bool isSwitch() noexcept { return false; }

  [[nodiscard]] static constexpr bool isList() noexcept { return true; }

  [[nodiscard]] static constexpr bool allowsEmpty() noexcept { return false; }
};

// ——— Positionals ———

template<Assignable T, FixedString Name>
class Positional : public detail::Arg<T>, public detail::PositionalName<Name>
{
public:
  constexpr explicit Positional(const std::string_view desc = "", T default_value = T{})
    : detail::Arg<T>(std::move(default_value), desc)
  {
  }

  [[nodiscard]] static constexpr bool isRequired() noexcept { return false; }

  [[nodiscard]] static constexpr bool isList() noexcept { return false; }

  [[nodiscard]] constexpr explicit operator bool() const noexcept { return this->isSet(); }
};

template<Assignable T, FixedString Name>
class RequiredPositional : public detail::Arg<T>, public detail::PositionalName<Name>
{
public:
  constexpr explicit RequiredPositional(const std::string_view desc = "") : detail::Arg<T>(desc) {}

  [[nodiscard]] static constexpr bool isRequired() noexcept { return true; }

  [[nodiscard]] static constexpr bool isList() noexcept { return false; }
};

template<typename T, FixedString Name>
class ListPositional : public detail::ListArg<T>, public detail::PositionalName<Name>
{
public:
  constexpr explicit ListPositional(const std::string_view desc = "") : detail::ListArg<T>(desc) {}

  [[nodiscard]] static constexpr bool isRequired() noexcept { return false; }

  [[nodiscard]] static constexpr bool isList() noexcept { return true; }

  [[nodiscard]] constexpr explicit operator bool() const noexcept { return this->isSet(); }
};

} // namespace zc::cli

// ——— Declaration macros (member `name_` + CLI name `snake-to-kebab`) ———

#define ZC_FLAG(name, sn, desc)                                         \
  ::zc::cli::Flag<::zc::cli::detail::snake_to_kebab(#name), sn> name##_ \
  {                                                                     \
    desc                                                                \
  }

#define ZC_OPTION(name, type, sn, desc, ...)                                    \
  ::zc::cli::Option<type, ::zc::cli::detail::snake_to_kebab(#name), sn> name##_ \
  {                                                                             \
    __VA_ARGS__, desc                                                           \
  }

#define ZC_REQUIRED_OPTION(name, type, sn, desc)                                        \
  ::zc::cli::RequiredOption<type, ::zc::cli::detail::snake_to_kebab(#name), sn> name##_ \
  {                                                                                     \
    desc                                                                                \
  }

#define ZC_LIST_OPTION(name, type, sn, desc)                                        \
  ::zc::cli::ListOption<type, ::zc::cli::detail::snake_to_kebab(#name), sn> name##_ \
  {                                                                                 \
    desc                                                                            \
  }

#define ZC_REQUIRED_LIST_OPTION(name, type, sn, desc)                                       \
  ::zc::cli::RequiredListOption<type, ::zc::cli::detail::snake_to_kebab(#name), sn> name##_ \
  {                                                                                         \
    desc                                                                                    \
  }

#define ZC_POSITIONAL(name, type, desc, ...)                                    \
  ::zc::cli::Positional<type, ::zc::cli::detail::snake_to_kebab(#name)> name##_ \
  {                                                                             \
    desc __VA_OPT__(, ) __VA_ARGS__                                             \
  }

#define ZC_REQUIRED_POSITIONAL(name, type, desc)                                        \
  ::zc::cli::RequiredPositional<type, ::zc::cli::detail::snake_to_kebab(#name)> name##_ \
  {                                                                                     \
    desc                                                                                \
  }

#define ZC_LIST_POSITIONAL(name, type, desc)                                        \
  ::zc::cli::ListPositional<type, ::zc::cli::detail::snake_to_kebab(#name)> name##_ \
  {                                                                                 \
    desc                                                                            \
  }
