#pragma once

#include <charconv>
#include <tuple>

#include "ArgStream.hpp"
#include "FixedString.hpp"
#include "framework/ui/UI.hpp"
#include "framework/util/concepts.hpp"
#include "framework/util/helpers.hpp"

namespace zc::cli::detail
{

template<typename T>
struct is_vector : std::false_type {};

template<typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc>> : std::true_type {};

template<typename T>
inline constexpr bool is_vector_v = is_vector<std::remove_cvref_t<T>>::value;

template<FixedString LongFlag, char ShortFlag, typename OptsTuple>
consteval bool has_conflicting_option()
{
  return []<typename... Opts>(std::type_identity<std::tuple<Opts...>>) {
    return (
      (Opts::longName() == LongFlag.sv() || (ShortFlag != '\0' && Opts::shortName() == ShortFlag)) || ...
    );
  }(std::type_identity<OptsTuple>{});
}

template<FixedString Name, typename PosTuple>
consteval bool has_conflicting_positional()
{
  return []<typename... Poss>(std::type_identity<std::tuple<Poss...>>) {
    return ((Poss::name() == Name.sv()) || ...);
  }(std::type_identity<PosTuple>{});
}

template<typename PosTuple>
consteval bool has_optional_positional()
{
  return []<typename... Poss>(std::type_identity<std::tuple<Poss...>>) {
    return ((!Poss::isRequired()) || ...);
  }(std::type_identity<PosTuple>{});
}

template<typename PosTuple>
consteval bool has_variadic_positional()
{
  return []<typename... Poss>(std::type_identity<std::tuple<Poss...>>) {
    return (is_vector_v<decltype(Poss::target)> || ...);
  }(std::type_identity<PosTuple>{});
}

template<typename SubCmd, typename SubsTuple>
consteval bool has_conflicting_subcommand()
{
  return []<typename... Subs>(std::type_identity<std::tuple<Subs...>>) {
    return ((Subs::name() == SubCmd::name()) || ...);
  }(std::type_identity<SubsTuple>{});
}

template<typename TupleA, typename TupleB>
consteval bool has_conflicting_option_tuples()
{
  return []<typename... OptsA>(std::type_identity<std::tuple<OptsA...>>) {
    return (
      []<typename OptA, typename... OptsB>(
        std::type_identity<OptA>, std::type_identity<std::tuple<OptsB...>>
      ) {
        return (
          (OptA::longName() == OptsB::longName() ||
           (OptA::shortName() != '\0' && OptA::shortName() == OptsB::shortName())) ||
          ...
        );
      }(std::type_identity<OptsA>{}, std::type_identity<TupleB>{}) ||
      ...
    );
  }(std::type_identity<TupleA>{});
}

template<typename Tuple>
void reset_matched(Tuple &items)
{
  std::apply([](auto &...item) { ((item.matched = false), ...); }, items);
}

template<Assignable T>
bool parse_value(std::string_view str, T &out)
{
  if constexpr (is_vector_v<T>) {
    typename T::value_type elem{};
    if (!parse_value(str, elem))
      return false;
    out.push_back(std::move(elem));
    return true;
  } else if constexpr (std::is_assignable_v<T &, std::string_view>) {
    out = str;
    return true;
  } else if constexpr (std::is_same_v<T, bool>) {
    out = to_bool(str);
    return true;
  } else if constexpr (std::is_arithmetic_v<T>) {
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), out);
    return ec == std::errc{};
  }
  return false;
}

template<typename Opt>
bool try_option(Opt &opt, ArgStream &args)
{
  using T = std::remove_reference_t<decltype(opt.target)>;

  const auto arg = args.current();
  const auto ln  = opt.longName();
  const char sn  = opt.shortName();

  if (arg.starts_with("--")) {
    const std::string_view body = arg.substr(2);
    // Long flag with '=' : --jobs=4
    if (body.starts_with(ln) && body.size() > ln.size() && body[ln.size()] == '=') {
      if (const std::string_view val = body.substr(ln.size() + 1); !parse_value(val, opt.target))
        return false;
      opt.matched = true;
      args.advance();
      return true;
    }
    if (body != ln)
      return false;
  } else {
    if (!opt.hasShortName() || arg.size() < 2 || arg.at(0) != '-' || arg.at(1) != sn)
      return false;

    if (arg.size() >= 3) {
      // Short flag with '=' : -j=4
      if (arg.at(2) == '=') {
        if (const std::string_view val = arg.substr(3); !parse_value(val, opt.target))
          return false;
        opt.matched = true;
        args.advance();
        return true;
      }
      // For non-booleans : Short flag without '=' : -j4
      if constexpr (!std::is_same_v<T, bool>) {
        if (const std::string_view val = arg.substr(2); parse_value(val, opt.target)) {
          opt.matched = true;
          args.advance();
          return true;
        }
      }
      return false;
    }
  }

  if constexpr (std::is_same_v<T, bool>) {
    opt.target  = true;
    opt.matched = true;
    args.advance();
    return true;
  } else {
    if (opt.hasImplicitValue()) {
      if (const std::string_view next = args.peek(); !next.empty() && !is_flag(next)) {
        if (T parsed_val{}; parse_value(next, parsed_val)) {
          opt.target  = std::move(parsed_val);
          opt.matched = true;
          args.advance(2);
          return true;
        }
      }
      opt.target  = opt.implicitValue();
      opt.matched = true;
      args.advance(1);
      return true;
    }
    if (const std::string_view val = args.peek(); val.empty() || !parse_value(val, opt.target))
      return false;
    opt.matched = true;
    args.advance(2);
    return true;
  }
}

/// @brief Try to find a match between command-line arguments and given options/flags
/// @tparam Tuple The type of the options tuple
/// @param options The options to try to match
/// @param args The command-line arguments to parse
/// @return Whether it found a matching flag
template<typename Tuple>
bool try_options_from(Tuple &options, ArgStream &args)
{
  return std::apply([&args](auto &...opt) { return (try_option(opt, args) || ...); }, options);
}

template<typename Tuple1, typename Tuple2>
bool try_combined_flags(Tuple1 &opts, Tuple2 &globs, const std::string_view arg)
{
  if (arg.size() < 3 || arg.at(0) != '-' || arg.at(1) == '-')
    return false;

  const std::string_view chars = arg.substr(1);

  // Also check that all flags exist and are boolean flags
  for (char c : chars) {
    bool found      = false;
    auto check_flag = [&](auto &opt) {
      using T = std::remove_reference_t<decltype(opt.target)>;
      if constexpr (std::is_same_v<T, bool>) {
        if (opt.shortName() == c)
          found = true;
      }
    };
    std::apply([&](auto &...o) { (check_flag(o), ...); }, opts);
    if (!found)
      std::apply([&](auto &...o) { (check_flag(o), ...); }, globs);

    if (!found)
      return false;
  }

  // Activate all found flags
  for (char c : chars) {
    auto activate_flag = [&](auto &opt) {
      using T = std::remove_reference_t<decltype(opt.target)>;
      if constexpr (std::is_same_v<T, bool>) {
        if (opt.shortName() == c) {
          opt.target  = true;
          opt.matched = true;
        }
      }
    };
    std::apply([&](auto &...o) { (activate_flag(o), ...); }, opts);
    std::apply([&](auto &...o) { (activate_flag(o), ...); }, globs);
  }

  return true;
}

template<typename Tuple>
bool try_positional(Tuple &poss, const std::string_view item)
{
  bool assigned = false;
  std::apply(
    [&](auto &...pos) {
      auto try_assign = [&](auto &p) {
        using TargetT = std::remove_reference_t<decltype(p.target)>;
        if (!assigned && (!p.matched || is_vector_v<TargetT>)) {
          if (parse_value(item, p.target)) {
            p.matched = true;
            assigned  = true;
          }
        }
      };
      (try_assign(pos), ...);
    },
    poss
  );
  return assigned;
}

template<typename Tuple>
void check_required_options(const Tuple &items)
{
  std::apply(
    [](const auto &...opt) {
      auto check_one = []<typename T>(const T &o) {
        using OptType = std::remove_cvref_t<T>;
        if constexpr (OptType::isRequired()) {
          if (!o.matched) {
            throw_or_terminate(
              Exception(CliError::MissingFlag, "missing required option '--{}'", o.longName())
            );
          }
        }
      };
      (check_one(opt), ...);
    },
    items
  );
}

template<typename Tuple>
void check_required_positionals(const Tuple &pos_items)
{
  std::apply(
    [](const auto &...pos) {
      auto check_one = []<typename T>(const T &p) {
        using PosType = std::remove_cvref_t<T>;
        if constexpr (PosType::isRequired()) {
          if (!p.matched) {
            throw_or_terminate(
              Exception(CliError::MissingPositional, "missing required argument '{}'", p.name())
            );
          }
        }
      };
      (check_one(pos), ...);
    },
    pos_items
  );
}

// UI
template<typename Tuple>
void print_pos_usage(const Tuple &items)
{
  std::apply(
    [](const auto &...pos) {
      auto print_one = []<typename T>(const T &p) {
        using PosType                   = std::remove_cvref_t<T>;
        using TargetT                   = std::remove_reference_t<decltype(p.target)>;
        constexpr std::string_view dots = is_vector_v<TargetT> ? "..." : "";
        if constexpr (PosType::isRequired())
          ui.print(" <{}>{}", p.name(), dots);
        else
          ui.print(" [<{}>{}]", p.name(), dots);
      };
      (print_one(pos), ...);
    },
    items
  );
}

template<typename Pos>
void print_pos_row(const Pos &pos)
{
  using PosType   = std::remove_cvref_t<Pos>;
  std::string col = std::format("  <{}>", pos.name());

  if constexpr (PosType::isRequired()) {
    std::string help_str = std::format("{} [required]", pos.help);
    ui.println("{:<32} {}", col, help_str);
  } else {
    ui.println("{:<32} {}", col, pos.help);
  }
}

template<typename Tuple>
void print_all_pos(const Tuple &items)
{
  std::apply([](const auto &...pos) { (print_pos_row(pos), ...); }, items);
}

template<typename Opt>
void print_opt_row(const Opt &opt)
{
  using T       = std::remove_reference_t<decltype(opt.target)>;
  using OptType = std::remove_cvref_t<Opt>;

  const auto ln = opt.longName();
  const auto sn = opt.shortName();

  std::string col = std::format("  --{}", ln);
  if (opt.hasShortName())
    std::format_to(std::back_inserter(col), ", -{}", sn);

  if constexpr (std::is_same_v<T, stdfs::path>)
    col += " <path>";
  else if constexpr (std::is_assignable_v<T &, std::string_view> || is_vector_v<T>)
    col += " <text>";
  else if constexpr (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
    col += " <value>";

  if constexpr (OptType::isRequired()) {
    std::string help_str = std::format("{} [required]", opt.help);
    ui.println("{:<32} {}", col, help_str);
  } else {
    ui.println("{:<32} {}", col, opt.help);
  }
}

template<typename Tuple>
void print_all_opts(const Tuple &items)
{
  std::apply([](const auto &...opt) { (print_opt_row(opt), ...); }, items);
}

template<typename Cmd>
void print_cmd_row(const Cmd & /*unused*/)
{
  ui.println("  {:<30} {}", Cmd::name(), Cmd::desc());
}

template<typename Tuple>
void print_all_cmds(const Tuple &items)
{
  std::apply([](const auto &...cmd) { (print_cmd_row(cmd), ...); }, items);
}

} // namespace zc::cli::detail
