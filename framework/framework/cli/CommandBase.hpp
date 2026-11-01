#pragma once

#include <string_view>
#include <tuple>
#include <type_traits>

#include "framework/cli/ArgStream.hpp"
#include "framework/cli/detail.hpp"
#include "framework/cli/FixedString.hpp"
#include "framework/Exception.hpp"
#include "framework/ui/Style.hpp"
#include "framework/ui/UI.hpp"
#include "framework/util/concepts.hpp"
#include "framework/util/helpers.hpp"

namespace zc::cli
{

/// @brief Base class of self-describing commands.
///
/// Subclasses declare their flags and positionals (see Args.hpp) as members, and expose them in
/// registration order through `options()` returning a tuple of references.
/// Intermediate base classes can define `allArgs(this Self &self)` to append shared arguments.
template<FixedString Name, FixedString Desc, Command... SubCmds>
class CommandBase : public CommandTag
{
  static_assert(!Name.sv().empty(), "Command name cannot be empty");
  static_assert(!detail::has_duplicate_subcommands<SubCmds...>(), "Duplicate subcommand name");

  [[no_unique_address]] std::tuple<SubCmds...> subcommands_{};

protected:
  CommandBase() = default;

public:
  [[nodiscard]] static constexpr std::string_view name() noexcept { return Name.sv(); }

  [[nodiscard]] static constexpr std::string_view desc() noexcept { return Desc.sv(); }

  [[nodiscard]] auto &subcommands() noexcept { return subcommands_; }

  [[nodiscard]] const auto &subcommands() const noexcept { return subcommands_; }

  template<Command Sub>
  [[nodiscard]] Sub &subcommand() noexcept
  {
    return std::get<Sub>(subcommands_);
  }

  template<Command Sub>
  [[nodiscard]] const Sub &subcommand() const noexcept
  {
    return std::get<Sub>(subcommands_);
  }

  /// @brief Every argument of the command, in registration order
  template<typename Self>
  [[nodiscard]] auto allArgs(this Self &self)
  {
    if constexpr (requires { self.options(); })
      return self.options();
    else
      return std::tuple<>{};
  }

  template<typename Self>
  bool parseStream(
    this Self &self, ArgStream &args, const bool execute_action = false,
    const std::string_view cmd_path = name(), const bool show_version_in_help = false
  )
  {
    constexpr bool has_subs    = sizeof...(SubCmds) > 0;
    constexpr bool require_sub = detail::requires_subcommand_v<Self>;
    static_assert(
      !require_sub || has_subs, "requires_subcommand cannot be enabled on a command without subcommands"
    );

    auto items = self.allArgs();
    detail::validate_args<decltype(items)>();
    detail::reset_matched(items);

    bool end_of_opts  = false;
    bool matched_leaf = false;
    while (args.hasNext()) {
      const std::string_view cur = args.current();

      if (!end_of_opts && cur == "--") {
        end_of_opts = true;
        args.advance();
      } else if (!end_of_opts && (cur == "--help" || cur == "-h")) {
        self.printHelp(cmd_path, show_version_in_help);
        return false;
      } else if (!end_of_opts && is_flag(cur)) {
        detail::consume_flag_token(items, args);
      } else if (!end_of_opts &&
                 detail::try_dispatch_subcommand(
                   self.subcommands(), args, execute_action, cmd_path, matched_leaf
                 )) {
        return matched_leaf;
      } else {
        detail::consume_positional_token<require_sub>(items, args);
      }
    }

    if constexpr (require_sub) {
      throw_or_terminate(Exception(CliError::MissingSubCommand, "subcommand required for '{}'", cmd_path));
    } else {
      detail::check_required_args(items);
      if (execute_action)
        self.execute();
      return true;
    }
  }

  template<typename Self>
  void printHelp(
    this const Self &self, const std::string_view cmd_path = name(), const bool show_version_in_help = false
  )
  {
    const auto items = const_cast<std::remove_const_t<Self> &>(self).allArgs();
    detail::validate_args<decltype(items)>();

    constexpr bool has_subs    = sizeof...(SubCmds) > 0;
    constexpr bool require_sub = detail::requires_subcommand_v<Self>;

    ui.println(Style::Underlined, "\nUsage:");
    if constexpr (has_subs)
      ui.print(require_sub ? "  {} <command> [options]" : "  {} [command] [options]", cmd_path);
    else
      ui.print("  {} [options]", cmd_path);
    detail::print_pos_usage(items);
    ui.println("\n\n{}\n", desc());

    if constexpr (detail::has_positionals_v<decltype(items)>) {
      ui.println(Style::Underlined, "Arguments:");
      detail::print_all_pos(items);
      ui.ln();
    }

    ui.println(Style::Underlined, "Options:");
    ui.println("{:<32} {}", "  --help, -h", "Show this help message and exit");
    if (show_version_in_help)
      ui.println("{:<32} {}", "  --version, -V", "Show version information and exit");
    detail::print_all_opts(items);

    if constexpr (has_subs) {
      ui.println(Style::Underlined, "\nCommands:");
      detail::print_all_cmds(self.subcommands());
    }
    ui.ln();
  }
};

/// @brief Convenience command class for pure dispatcher / root commands that require a subcommand
template<FixedString Name, FixedString Desc, Command... SubCmds>
class RootCommand : public CommandBase<Name, Desc, SubCmds...>
{
public:
  static constexpr bool requires_subcommand = true;

  RootCommand() = default;
};

} // namespace zc::cli
