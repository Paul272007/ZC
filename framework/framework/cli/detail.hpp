#pragma once

#include <charconv>
#include <tuple>

#include "ArgStream.hpp"
#include "framework/ui/UI.hpp"
#include "framework/util/concepts.hpp"
#include "framework/util/helpers.hpp"

namespace zc::cli::detail
{

template<typename Tuple>
void reset_matched(Tuple &items)
{
  std::apply([](auto &...item) { ((item.matched = false), ...); }, items);
}

template<Assignable T>
bool parse_value(std::string_view str, T &out)
{
  if constexpr (std::is_same_v<T, std::string_view>) {
    out = str;
    return true;
  } else if constexpr (std::is_same_v<T, bool>) {
    out = to_bool(str);
    return true;
  } else if constexpr (std::is_arithmetic_v<T>) {
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), out);
    return ec == std::errc{};
  }
  // TODO: Add other types / complex types
  return false;
}

// TODO: simplify / refactor
template<typename Opt>
bool try_option(Opt &opt, ArgStream &args)
{
  using T = std::remove_reference_t<decltype(opt.target)>;

  const auto arg = args.current();
  const auto ln  = opt.longName();
  const char sn  = opt.shortName();

  // Long flag with '=' : --jobs=4
  if (opt.hasLongName() && arg.starts_with(ln) && arg.size() > ln.size() && arg[ln.size()] == '=') {
    if (const std::string_view val = arg.substr(ln.size() + 1); !parse_value(val, opt.target))
      return false;
    opt.matched = true;
    args.advance();
    return true;
  }

  // Short flag with '=' : -j=4
  if (opt.hasShortName() && arg.size() >= 3 && arg.at(0) == '-' && arg.at(1) == sn && arg.at(2) == '=') {
    if (const std::string_view val = arg.substr(3); !parse_value(val, opt.target))
      return false;
    opt.matched = true;
    args.advance();
    return true;
  }

  // For non-booleans : Short flag without '=' : -j4
  if constexpr (!std::is_same_v<T, bool>) {
    if (opt.hasShortName() && arg.size() >= 3 && arg.at(0) == '-' && arg.at(1) == sn && arg.at(2) != '-') {
      if (const std::string_view val = arg.substr(2); parse_value(val, opt.target)) {
        opt.matched = true;
        args.advance();
        return true;
      }
    }
  }

  // Exact match: --jobs or -j
  const bool is_long  = arg == ln;
  const bool is_short = opt.hasShortName() && arg.size() == 2 && arg.at(0) == '-' && arg.at(1) == sn;

  if (!is_long && !is_short)
    return false;

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
        if (!assigned && !p.matched) {
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
              Exception(CliError::MissingFlag, "missing required option '{}'", o.longName())
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
        using PosType = std::remove_cvref_t<T>;
        if constexpr (PosType::isRequired())
          ui.print(" <{}>", p.name());
        else
          ui.print(" [<{}>]", p.name());
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

  std::string col = std::format("  {}", ln);
  if (opt.hasShortName())
    std::format_to(std::back_inserter(col), ", -{}", sn);

  if constexpr (std::is_same_v<T, std::string_view>)
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
  ui.println("  {:<20} {}", Cmd::name(), Cmd::desc());
}

template<typename Tuple>
void print_all_cmds(const Tuple &items)
{
  std::apply([](const auto &...cmd) { (print_cmd_row(cmd), ...); }, items);
}

} // namespace zc::cli::detail
