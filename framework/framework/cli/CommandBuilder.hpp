#pragma once

#include "ArgStream.hpp"
#include "detail.hpp"
#include "FixedString.hpp"
#include "framework/ui/UI.hpp"
#include "framework/util/concepts.hpp"
#include "Option.hpp"
#include "Positional.hpp"

namespace zc::cli
{

template<
  FixedString Name, FixedString Desc, typename OptsTuple = std::tuple<>, typename GlobsTuple = std::tuple<>,
  typename SubsTuple = std::tuple<>, typename PosTuple = std::tuple<>,
  std::invocable Action = decltype([] {})
>
struct CommandBuilder : CommandTag {
  OptsTuple  opts;
  GlobsTuple globs;
  SubsTuple  subs;
  PosTuple   poss;
  Action     action;
  bool       require_subcommand = false;

  [[nodiscard]] static constexpr std::string_view name() { return Name.sv(); }

  [[nodiscard]] static constexpr std::string_view desc() { return Desc.sv(); }

  template<FixedString LongFlag, char ShortFlag = '\0', Assignable T>
  [[nodiscard]] auto flag(T &target, const std::string_view help = "")
  {
    auto new_opt  = opt<T, LongFlag, ShortFlag>(target, help);
    auto new_opts = std::tuple_cat(std::move(opts), std::make_tuple(new_opt));
    return CommandBuilder<Name, Desc, decltype(new_opts), GlobsTuple, SubsTuple, PosTuple, Action>{
      .opts               = std::move(new_opts),
      .globs              = std::move(globs),
      .subs               = std::move(subs),
      .poss               = std::move(poss),
      .action             = std::move(action),
      .require_subcommand = require_subcommand,
    };
  }

  template<FixedString LongFlag, char ShortFlag = '\0', Assignable T>
  [[nodiscard]] auto flagWithDefault(T &target, T implicit_val, const std::string_view help = "")
  {
    auto new_opt  = opt_with_default<T, LongFlag, ShortFlag>(target, std::move(implicit_val), help);
    auto new_opts = std::tuple_cat(std::move(opts), std::make_tuple(new_opt));
    return CommandBuilder<Name, Desc, decltype(new_opts), GlobsTuple, SubsTuple, PosTuple, Action>{
      .opts               = std::move(new_opts),
      .globs              = std::move(globs),
      .subs               = std::move(subs),
      .poss               = std::move(poss),
      .action             = std::move(action),
      .require_subcommand = require_subcommand,
    };
  }

  template<FixedString LongFlag, char ShortFlag = '\0', Assignable T>
  [[nodiscard]] auto requiredFlag(T &target, const std::string_view help = "")
  {
    auto new_opt  = required_opt<T, LongFlag, ShortFlag>(target, help);
    auto new_opts = std::tuple_cat(std::move(opts), std::make_tuple(new_opt));
    return CommandBuilder<Name, Desc, decltype(new_opts), GlobsTuple, SubsTuple, PosTuple, Action>{
      .opts               = std::move(new_opts),
      .globs              = std::move(globs),
      .subs               = std::move(subs),
      .poss               = std::move(poss),
      .action             = std::move(action),
      .require_subcommand = require_subcommand,
    };
  }

  template<FixedString LongFlag, char ShortFlag = '\0', Assignable T>
  [[nodiscard]] auto global(T &target, const std::string_view help = "")
  {
    auto new_glob  = opt<T, LongFlag, ShortFlag>(target, help);
    auto new_globs = std::tuple_cat(std::move(globs), std::make_tuple(new_glob));
    return CommandBuilder<Name, Desc, OptsTuple, decltype(new_globs), SubsTuple, PosTuple, Action>{
      .opts               = std::move(opts),
      .globs              = std::move(new_globs),
      .subs               = std::move(subs),
      .poss               = std::move(poss),
      .action             = std::move(action),
      .require_subcommand = require_subcommand,
    };
  }

  template<FixedString LongFlag, char ShortFlag = '\0', Assignable T>
  [[nodiscard]] auto globalWithDefault(T &target, T implicit_val, const std::string_view help = "")
  {
    auto new_glob  = opt_with_default<T, LongFlag, ShortFlag>(target, std::move(implicit_val), help);
    auto new_globs = std::tuple_cat(std::move(globs), std::make_tuple(new_glob));
    return CommandBuilder<Name, Desc, OptsTuple, decltype(new_globs), SubsTuple, PosTuple, Action>{
      .opts               = std::move(opts),
      .globs              = std::move(new_globs),
      .subs               = std::move(subs),
      .poss               = std::move(poss),
      .action             = std::move(action),
      .require_subcommand = require_subcommand,
    };
  }

  template<FixedString LongFlag, char ShortFlag = '\0', Assignable T>
  [[nodiscard]] auto requiredGlobal(T &target, const std::string_view help = "")
  {
    auto new_glob  = required_opt<T, LongFlag, ShortFlag>(target, help);
    auto new_globs = std::tuple_cat(std::move(globs), std::make_tuple(new_glob));
    return CommandBuilder<Name, Desc, OptsTuple, decltype(new_globs), SubsTuple, PosTuple, Action>{
      .opts               = std::move(opts),
      .globs              = std::move(new_globs),
      .subs               = std::move(subs),
      .poss               = std::move(poss),
      .action             = std::move(action),
      .require_subcommand = require_subcommand,
    };
  }

  template<FixedString PosName, Assignable T>
  [[nodiscard]] auto positional(T &target, const std::string_view help = "")
  {
    auto new_pos  = pos<T, PosName>(target, help);
    auto new_poss = std::tuple_cat(std::move(poss), std::make_tuple(new_pos));
    return CommandBuilder<Name, Desc, OptsTuple, GlobsTuple, SubsTuple, decltype(new_poss), Action>{
      .opts               = std::move(opts),
      .globs              = std::move(globs),
      .subs               = std::move(subs),
      .poss               = std::move(new_poss),
      .action             = std::move(action),
      .require_subcommand = require_subcommand,
    };
  }

  template<FixedString PosName, Assignable T>
  [[nodiscard]] auto requiredPositional(T &target, const std::string_view help = "")
  {
    auto new_pos  = required_pos<T, PosName>(target, help);
    auto new_poss = std::tuple_cat(std::move(poss), std::make_tuple(new_pos));
    return CommandBuilder<Name, Desc, OptsTuple, GlobsTuple, SubsTuple, decltype(new_poss), Action>{
      .opts               = std::move(opts),
      .globs              = std::move(globs),
      .subs               = std::move(subs),
      .poss               = std::move(new_poss),
      .action             = std::move(action),
      .require_subcommand = require_subcommand,
    };
  }

  template<Command Cmd>
  [[nodiscard]] auto subcommand(Cmd sub)
  {
    auto new_subs = std::tuple_cat(std::move(subs), std::make_tuple(std::move(sub)));
    return CommandBuilder<Name, Desc, OptsTuple, GlobsTuple, decltype(new_subs), PosTuple, Action>{
      .opts               = std::move(opts),
      .globs              = std::move(globs),
      .subs               = std::move(new_subs),
      .poss               = std::move(poss),
      .action             = std::move(action),
      .require_subcommand = require_subcommand,
    };
  }

  template<std::invocable NewAction>
  [[nodiscard]] auto setAction(NewAction a)
  {
    return CommandBuilder<Name, Desc, OptsTuple, GlobsTuple, SubsTuple, PosTuple, NewAction>{
      .opts               = std::move(opts),
      .globs              = std::move(globs),
      .subs               = std::move(subs),
      .poss               = std::move(poss),
      .action             = std::move(a),
      .require_subcommand = require_subcommand,
    };
  }

  [[nodiscard]] auto requiresSubcommand(bool req = true)
  {
    auto copy               = *this;
    copy.require_subcommand = req;
    return copy;
  }

  template<typename IncomingGlobsTuple>
  void parseStream(ArgStream &args, IncomingGlobsTuple &incoming_globs)
  {
    detail::reset_matched(opts);
    detail::reset_matched(globs);
    detail::reset_matched(poss);

    auto all_globs   = std::tuple_cat(incoming_globs, globs);
    bool end_of_opts = false;

    while (args.hasNext()) {
      std::string_view cur = args.current();

      // No more flags
      if (!end_of_opts && cur == "--") {
        end_of_opts = true;
        args.advance();
        continue;
      }

      // Help menu
      if (!end_of_opts && (cur == "--help" || cur == "-h")) {
        printHelp(incoming_globs);
        return;
      }

      // Other flags
      if (!end_of_opts && is_flag(cur)) {
        if (detail::try_options_from(opts, args) || detail::try_options_from(all_globs, args)) {
          // matched and consumed
        } else if (detail::try_combined_flags(opts, all_globs, cur)) {
          args.advance();
        } else {
          throw_or_terminate(Exception(CliError::UnknownFlag, "unknown option '{}'", cur));
        }
      } else {
        // Try subcommand
        if (!end_of_opts) {
          bool dispatched = false;

          std::apply(
            [&](auto &...sub) {
              auto try_sub = [&](auto &s) {
                if (!dispatched && args.current() == s.name()) {
                  dispatched = true;
                  args.advance();
                  s.parseStream(args, all_globs);
                }
              };
              (try_sub(sub), ...);
            },
            subs
          );

          if (dispatched)
            return;
        }

        if (detail::try_positional(poss, cur))
          args.advance();
        else
          throw_or_terminate(Exception(CliError::UnexpectedPositional, "unexpected argument '{}'", cur));
      }
    }

    // Check if required subcommand is missing
    if constexpr (std::tuple_size_v<SubsTuple> > 0) {
      if (require_subcommand)
        throw_or_terminate(Exception(CliError::MissingSubCommand, "subcommand required for '{}'", name()));
    }

    // Check if required option is missing
    detail::check_required_options(opts);
    detail::check_required_options(globs);

    // Check if required positional is missing
    detail::check_required_positionals(poss);

    action(); // all tokens validated and consumed
  }

  template<typename IncomingGlobsTuple>
  void printHelp(const IncomingGlobsTuple &incoming_globs) const
  {
    constexpr bool has_subcommands = std::tuple_size_v<SubsTuple> > 0;
    // Usage
    ui.println_underlined("Usage:");
    ui.print("  {} [options]", name());
    if constexpr (has_subcommands)
      ui.print(" <command>");
    ui.println("\n\n{}\n", desc());

    // Command arguments
    if constexpr (std::tuple_size_v<PosTuple> > 0) {
      ui.println_underlined("Arguments:");
      detail::print_all_pos(poss);
      ui.ln();
    }

    // Options
    ui.println_underlined("Options:");
    ui.println("{:<32} {}", "  --help, -h", "Show this help message and exit");
    detail::print_all_opts(opts);
    ui.ln();

    // Global options
    if constexpr (const auto all_globs = std::tuple_cat(incoming_globs, globs);
                  std::tuple_size_v<decltype(all_globs)> > 0) {
      ui.println("Global options:");
      detail::print_all_opts(all_globs);
    }
    if constexpr (has_subcommands) {
      ui.println("\nCommands:");
      detail::print_all_cmds(subs);
    }
  }
};

template<FixedString Name, FixedString Desc>
constexpr auto cmd()
{
  return CommandBuilder<Name, Desc>{};
}

} // namespace zc::cli
