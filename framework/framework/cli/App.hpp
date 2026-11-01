#pragma once

#include <string_view>

#include "framework/cli/ArgStream.hpp"
#include "framework/ui/UI.hpp"
#include "framework/util/concepts.hpp"

namespace zc::cli
{

template<Command Root>
class App
{
  Root             root_{};
  std::string_view version_;

public:
  constexpr explicit App(const std::string_view version = "") : version_(version) {}

  [[nodiscard]] Root &root() noexcept { return root_; }

  [[nodiscard]] const Root &root() const noexcept { return root_; }

  void printHelp() const { root_.printHelp(Root::name(), !version_.empty()); }

  bool parse(const int argc, const char * const *argv, const bool execute_action = false)
  {
    if (argc >= 2) {
      const std::string_view first = argv[1];
      if (!version_.empty() && (first == "--version" || first == "-V")) {
        ui.println("{}", version_);
        return false;
      }
      if (first == "--help" || first == "-h") {
        printHelp();
        return false;
      }
    }

    ArgStream stream{ .argc = argc, .argv = argv };
    return root_.parseStream(stream, execute_action, Root::name(), !version_.empty());
  }

  void run(const int argc, const char * const *argv) { (void)parse(argc, argv, true); }
};

} // namespace zc::cli
