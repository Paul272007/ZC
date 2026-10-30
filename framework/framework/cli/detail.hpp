#pragma once

#include <array>
#include <charconv>
#include <cstddef>
#include <format>
#include <optional>
#include <ranges>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "framework/cli/Args.hpp"
#include "framework/cli/ArgStream.hpp"
#include "framework/Exception.hpp"
#include "framework/fs.hpp"
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

template<typename T>
struct is_optional : std::false_type {};

template<typename T>
struct is_optional<std::optional<T>> : std::true_type {};

template<typename T>
inline constexpr bool is_optional_v = is_optional<std::remove_cvref_t<T>>::value;

template<typename T>
inline constexpr bool is_flag_arg_v = std::derived_from<std::remove_cvref_t<T>, FlagTag>;

template<typename T>
inline constexpr bool is_positional_arg_v = std::derived_from<std::remove_cvref_t<T>, PositionalTag>;

template<typename T>
inline constexpr bool requires_subcommand_v =
  requires { requires static_cast<bool>(std::remove_cvref_t<T>::requires_subcommand); };

// ——— Compile-time validation ———

struct ArgMeta {
  bool             is_pos{};
  bool             is_required{};
  bool             is_list{};
  std::string_view name{};
  char             short_name{ '\0' };
};

template<typename T>
consteval ArgMeta make_arg_meta()
{
  using A = std::remove_cvref_t<T>;
  if constexpr (is_flag_arg_v<A>) {
    return ArgMeta{
      .is_pos      = false,
      .is_required = A::isRequired(),
      .is_list     = A::isList(),
      .name        = A::longName(),
      .short_name  = A::shortName(),
    };
  } else {
    return ArgMeta{
      .is_pos      = true,
      .is_required = A::isRequired(),
      .is_list     = A::isList(),
      .name        = A::name(),
      .short_name  = '\0',
    };
  }
}

template<std::size_t N>
consteval bool has_duplicate_flag_names(const std::array<ArgMeta, N> &metas)
{
  for (std::size_t i = 0; i < N; ++i) {
    if (metas[i].is_pos)
      continue;
    for (std::size_t j = i + 1; j < N; ++j) {
      if (metas[j].is_pos)
        continue;
      if (metas[i].name == metas[j].name)
        return true;
      if (metas[i].short_name != '\0' && metas[i].short_name == metas[j].short_name)
        return true;
    }
  }
  return false;
}

template<std::size_t N>
consteval bool has_duplicate_positional_names(const std::array<ArgMeta, N> &metas)
{
  for (std::size_t i = 0; i < N; ++i) {
    if (!metas[i].is_pos)
      continue;
    for (std::size_t j = i + 1; j < N; ++j)
      if (metas[j].is_pos && metas[i].name == metas[j].name)
        return true;
  }
  return false;
}

template<std::size_t N>
consteval bool has_flag_positional_conflict(const std::array<ArgMeta, N> &metas)
{
  for (std::size_t i = 0; i < N; ++i) {
    for (std::size_t j = i + 1; j < N; ++j)
      if (metas[i].is_pos != metas[j].is_pos && metas[i].name == metas[j].name)
        return true;
  }
  return false;
}

template<std::size_t N>
consteval bool has_required_after_optional_positional(const std::array<ArgMeta, N> &metas)
{
  bool seen_optional = false;
  for (std::size_t i = 0; i < N; ++i) {
    if (!metas[i].is_pos)
      continue;
    if (metas[i].is_required && seen_optional)
      return true;
    if (!metas[i].is_required)
      seen_optional = true;
  }
  return false;
}

template<std::size_t N>
consteval bool has_positional_after_list_positional(const std::array<ArgMeta, N> &metas)
{
  bool seen_list = false;
  for (std::size_t i = 0; i < N; ++i) {
    if (!metas[i].is_pos)
      continue;
    if (seen_list)
      return true;
    if (metas[i].is_list)
      seen_list = true;
  }
  return false;
}

template<typename ArgsTuple>
consteval void validate_args()
{
  []<typename... Elems>(std::type_identity<std::tuple<Elems...>>) consteval {
    static_assert(
      ((is_flag_arg_v<Elems> || is_positional_arg_v<Elems>) && ...),
      "All elements returned by options()/allArgs() must be CLI argument types from Args.hpp"
    );

    constexpr std::array<ArgMeta, sizeof...(Elems)> metas{ make_arg_meta<Elems>()... };

    static_assert(!has_duplicate_flag_names(metas), "Duplicate option/flag name");
    static_assert(!has_duplicate_positional_names(metas), "Duplicate positional argument name");
    static_assert(
      !has_flag_positional_conflict(metas), "Option long name conflicts with a positional argument name"
    );
    static_assert(
      !has_required_after_optional_positional(metas),
      "Cannot declare a required positional argument after an optional positional argument"
    );
    static_assert(
      !has_positional_after_list_positional(metas),
      "Cannot declare a positional argument after a ListPositional argument"
    );
  }(std::type_identity<std::remove_cvref_t<ArgsTuple>>{});
}

template<typename... SubCmds>
consteval bool has_duplicate_subcommands()
{
  constexpr std::array<std::string_view, sizeof...(SubCmds)> names{ SubCmds::name()... };
  for (std::size_t i = 0; i < names.size(); ++i) {
    for (std::size_t j = i + 1; j < names.size(); ++j)
      if (names[i] == names[j])
        return true;
  }
  return false;
}

template<typename ArgsTuple>
inline constexpr bool has_positionals_v = []<typename... Elems>(std::type_identity<std::tuple<Elems...>>) {
  return (is_positional_arg_v<Elems> || ...);
}(std::type_identity<std::remove_cvref_t<ArgsTuple>>{});

// ——— Runtime parsing ———

template<typename Tuple>
void reset_matched(Tuple &items)
{
  std::apply(
    [](auto &...item) {
      auto reset_one = []<typename T>(T &arg) {
        using ArgType = std::remove_cvref_t<T>;
        arg.setMatched(false);
        if constexpr (ArgType::isList())
          arg.storage().clear();
      };
      (reset_one(item), ...);
    },
    items
  );
}

template<Assignable T>
bool parse_value(const std::string_view str, T &out)
{
  if constexpr (is_optional_v<T>) {
    if (!out)
      out.emplace();
    return parse_value(str, *out);
  } else if constexpr (is_vector_v<T>) {
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
    const char *end = str.data() + str.size();
    auto [ptr, ec]  = std::from_chars(str.data(), end, out);
    return ec == std::errc{} && ptr == end;
  }
  return false;
}

struct FlagMatch {
  bool                            matched    = false;
  std::optional<std::string_view> inline_val = std::nullopt;
  std::string_view                label{};
};

template<typename OptType>
[[nodiscard]] FlagMatch match_flag_token(const std::string_view arg) noexcept
{
  const std::string_view ln = OptType::longName();
  if (arg.starts_with("--")) {
    const std::string_view body = arg.substr(2);
    if (body == ln)
      return { .matched = true, .label = arg };
    if (body.starts_with(ln) && body.size() > ln.size() && body[ln.size()] == '=')
      return {
        .matched    = true,
        .inline_val = body.substr(ln.size() + 1),
        .label      = arg.substr(0, 2 + ln.size()),
      };
    return {};
  }

  if (!OptType::hasShortName() || arg.size() < 2 || arg[0] != '-' || arg[1] != OptType::shortName())
    return {};
  if (arg.size() == 2)
    return { .matched = true, .label = arg };
  if (arg[2] == '=')
    return { .matched = true, .inline_val = arg.substr(3), .label = arg.substr(0, 2) };
  if constexpr (!OptType::isSwitch() && !OptType::isList())
    return { .matched = true, .inline_val = arg.substr(2), .label = arg.substr(0, 2) };
  return {};
}

template<typename Opt>
void consume_inline_value(Opt &opt, const std::string_view val, const std::string_view label)
{
  using OptType = std::remove_cvref_t<Opt>;
  if constexpr (OptType::isList()) {
    bool any = false;
    for (const auto part : val | std::views::split(',') |
           std::views::filter([](auto &&w) { return !std::ranges::empty(w); })) {
      typename OptType::elem_type elem{};
      if (const std::string_view sv(part); !parse_value(sv, elem))
        throw_or_terminate(Exception(CliError::InvalidValue, "invalid value '{}' for '{}'", sv, label));
      opt.storage().push_back(std::move(elem));
      any = true;
    }
    if (!any && !OptType::allowsEmpty())
      throw_or_terminate(Exception(CliError::MissingValue, "missing value for '{}'", label));
  } else if (!parse_value(val, opt.storage())) {
    throw_or_terminate(Exception(CliError::InvalidValue, "invalid value '{}' for '{}'", val, label));
  }
}

template<typename Opt>
void consume_list_tokens(Opt &opt, ArgStream &args, const std::string_view label)
{
  using OptType = std::remove_cvref_t<Opt>;
  args.advance();

  bool any = false;
  while (args.hasNext()) {
    const std::string_view cur = args.current();
    if (cur == "--" || is_flag(cur))
      break;

    typename OptType::elem_type elem{};
    if (!parse_value(cur, elem))
      break;

    opt.storage().push_back(std::move(elem));
    any = true;
    args.advance();
  }

  if (!any && !OptType::allowsEmpty())
    throw_or_terminate(Exception(CliError::MissingValue, "missing value for '{}'", label));
}

template<typename Opt>
void consume_next_value(Opt &opt, ArgStream &args, const std::string_view label)
{
  if (args.pos + 1 >= args.argc)
    throw_or_terminate(Exception(CliError::MissingValue, "missing value for '{}'", label));

  if (const std::string_view val = args.peek(); !parse_value(val, opt.storage()))
    throw_or_terminate(Exception(CliError::InvalidValue, "invalid value '{}' for '{}'", val, label));

  args.advance(2);
}

template<typename Opt>
bool try_flag_arg(Opt &opt, ArgStream &args)
{
  using OptType = std::remove_cvref_t<Opt>;
  if constexpr (!is_flag_arg_v<OptType>) {
    return false;
  } else {
    const FlagMatch m = match_flag_token<OptType>(args.current());
    if (!m.matched)
      return false;

    if (m.inline_val) {
      consume_inline_value(opt, *m.inline_val, m.label);
      args.advance();
    } else if constexpr (OptType::isSwitch()) {
      opt.storage() = true;
      args.advance();
    } else if constexpr (OptType::isList()) {
      consume_list_tokens(opt, args, m.label);
    } else {
      consume_next_value(opt, args, m.label);
    }

    opt.setMatched(true);
    return true;
  }
}

template<typename Tuple>
bool try_flags_from(Tuple &items, ArgStream &args)
{
  return std::apply([&args](auto &...item) { return (try_flag_arg(item, args) || ...); }, items);
}

template<typename Tuple>
bool try_combined_flags(Tuple &items, const std::string_view arg)
{
  if (arg.size() < 3 || arg[0] != '-' || arg[1] == '-')
    return false;

  const std::string_view chars = arg.substr(1);

  for (const char c : chars) {
    bool found      = false;
    auto check_flag = [&]<typename T>(T & /*opt*/) {
      using OptType = std::remove_cvref_t<T>;
      if constexpr (is_flag_arg_v<OptType>) {
        if constexpr (OptType::isSwitch()) {
          if (OptType::shortName() == c)
            found = true;
        }
      }
    };
    std::apply([&](auto &...o) { (check_flag(o), ...); }, items);
    if (!found)
      return false;
  }

  for (const char c : chars) {
    auto activate_flag = [&]<typename T>(T &opt) {
      using OptType = std::remove_cvref_t<T>;
      if constexpr (is_flag_arg_v<OptType>) {
        if constexpr (OptType::isSwitch()) {
          if (OptType::shortName() == c) {
            opt.storage() = true;
            opt.setMatched(true);
          }
        }
      }
    };
    std::apply([&](auto &...o) { (activate_flag(o), ...); }, items);
  }

  return true;
}

template<typename Tuple>
void consume_flag_token(Tuple &items, ArgStream &args)
{
  const std::string_view cur = args.current();
  if (try_flags_from(items, args))
    return;
  if (try_combined_flags(items, cur)) {
    args.advance();
    return;
  }
  throw_or_terminate(Exception(CliError::UnknownFlag, "unknown option '{}'", cur));
}

template<typename SubsTuple>
bool try_dispatch_subcommand(
  SubsTuple &subs, ArgStream &args, const bool execute_action, const std::string_view cmd_path,
  bool &matched_leaf
)
{
  bool dispatched = false;
  std::apply(
    [&](auto &...sub) {
      auto try_one = [&](auto &s) {
        if (!dispatched && args.current() == s.name()) {
          dispatched = true;
          args.advance();
          matched_leaf = s.parseStream(args, execute_action, std::format("{} {}", cmd_path, s.name()));
        }
      };
      (try_one(sub), ...);
    },
    subs
  );
  return dispatched;
}

template<typename Tuple>
bool try_positional_from(Tuple &items, const std::string_view token)
{
  bool assigned = false;
  std::apply(
    [&](auto &...item) {
      auto try_assign = [&]<typename T>(T &p) {
        using PosType = std::remove_cvref_t<T>;
        if constexpr (is_positional_arg_v<PosType>) {
          if (!assigned && (!p.isSet() || PosType::isList())) {
            if (!parse_value(token, p.storage()))
              throw_or_terminate(Exception(
                CliError::InvalidValue, "invalid value '{}' for argument '{}'", token, PosType::name()
              ));
            p.setMatched(true);
            assigned = true;
          }
        }
      };
      (try_assign(item), ...);
    },
    items
  );
  return assigned;
}

template<bool RequireSub, typename Tuple>
void consume_positional_token(Tuple &items, ArgStream &args)
{
  const std::string_view cur = args.current();
  if (try_positional_from(items, cur)) {
    args.advance();
    return;
  }
  if constexpr (RequireSub)
    throw_or_terminate(Exception(CliError::UnknownSubCommand, "unknown subcommand '{}'", cur));
  else
    throw_or_terminate(Exception(CliError::UnexpectedPositional, "unexpected argument '{}'", cur));
}

template<typename Tuple>
void check_required_args(const Tuple &items)
{
  std::apply(
    [](const auto &...item) {
      auto check_one = []<typename T>(const T &arg) {
        using ArgType = std::remove_cvref_t<T>;
        if constexpr (is_flag_arg_v<ArgType> && ArgType::isRequired()) {
          if (!arg.isSet())
            throw_or_terminate(
              Exception(CliError::MissingFlag, "missing required option '--{}'", ArgType::longName())
            );
        } else if constexpr (is_positional_arg_v<ArgType> && ArgType::isRequired()) {
          if (!arg.isSet())
            throw_or_terminate(
              Exception(CliError::MissingPositional, "missing required argument '{}'", ArgType::name())
            );
        }
      };
      (check_one(item), ...);
    },
    items
  );
}

// ——— Help UI ———

template<typename Tuple>
void print_pos_usage(const Tuple &items)
{
  std::apply(
    [](const auto &...item) {
      auto print_one = []<typename T>(const T & /*p*/) {
        using PosType = std::remove_cvref_t<T>;
        if constexpr (is_positional_arg_v<PosType>) {
          constexpr std::string_view dots = PosType::isList() ? "..." : "";
          if constexpr (PosType::isRequired())
            ui.print(" <{}>{}", PosType::name(), dots);
          else
            ui.print(" [<{}>{}]", PosType::name(), dots);
        }
      };
      (print_one(item), ...);
    },
    items
  );
}

template<typename Pos>
void print_pos_row(const Pos &pos)
{
  using PosType   = std::remove_cvref_t<Pos>;
  std::string col = std::format("  <{}>", PosType::name());

  if constexpr (PosType::isRequired())
    ui.println("{:<32} {} [required]", col, pos.description());
  else
    ui.println("{:<32} {}", col, pos.description());
}

template<typename Tuple>
void print_all_pos(const Tuple &items)
{
  std::apply(
    [](const auto &...item) {
      auto maybe_print = []<typename T>(const T &p) {
        if constexpr (is_positional_arg_v<T>)
          print_pos_row(p);
      };
      (maybe_print(item), ...);
    },
    items
  );
}

template<typename Opt>
void print_opt_row(const Opt &opt)
{
  using OptType = std::remove_cvref_t<Opt>;

  std::string col = std::format("  --{}", OptType::longName());
  if constexpr (OptType::hasShortName())
    std::format_to(std::back_inserter(col), ", -{}", OptType::shortName());

  if constexpr (!OptType::isSwitch()) {
    if constexpr (OptType::isList()) {
      col += OptType::allowsEmpty() ? " [<args>...]" : " <args>...";
    } else {
      using ValT = typename OptType::value_type;
      if constexpr (std::is_same_v<ValT, stdfs::path>)
        col += " <path>";
      else if constexpr (std::is_assignable_v<ValT &, std::string_view>)
        col += " <text>";
      else if constexpr (std::is_arithmetic_v<ValT>)
        col += " <value>";
    }
  }

  if constexpr (OptType::isRequired())
    ui.println("{:<32} {} [required]", col, opt.description());
  else
    ui.println("{:<32} {}", col, opt.description());
}

template<typename Tuple>
void print_all_opts(const Tuple &items)
{
  std::apply(
    [](const auto &...item) {
      auto maybe_print = []<typename T>(const T &o) {
        if constexpr (is_flag_arg_v<T>)
          print_opt_row(o);
      };
      (maybe_print(item), ...);
    },
    items
  );
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
