#pragma once

#include "ArgStream.hpp"
#include "framework/ui/UI.hpp"
#include "framework/util/concepts.hpp"

namespace zc::cli
{

template<Command Cmd>
struct AppBuilder {
  Cmd              cmd;
  std::string_view version_{};

  [[nodiscard]] AppBuilder version(const std::string_view v) &&
  {
    return { .cmd = std::move(cmd), .version_ = v };
  }

  void printHelp() const { cmd.printHelp(std::tuple{}); }

  bool parse(const int argc, char **argv)
  {
    if (argc >= 2) {
      const std::string_view first = argv[1];
      if (!version_.empty() && (first == "--version" || first == "-V")) {
        ui.println("{}", version_);
        return true;
      }
      if (first == "--help" || first == "-h") {
        printHelp();
        return true;
      }
    }
    ArgStream    args{ .argc = argc, .argv = const_cast<const char **>(argv) };
    std::tuple<> empty;
    cmd.parseStream(args, empty);
    return true;
  }
};

template<typename Cmd>
auto app(Cmd &&cmd)
{
  return AppBuilder<std::remove_cvref_t<Cmd>>{ std::forward<Cmd>(cmd) };
}

} // namespace zc::cli

// zc
