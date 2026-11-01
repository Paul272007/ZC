// clang-format off
//
//   ________  ________
//  |\_____  \|\   ____\
//   \|___/  /\ \  \___|
//       /  / /\ \  \
//      /  /_/__\ \  \____
//     |\________\ \_______\
//      \|_______|\|_______|
//
// Copyright (c) 2026 Paul Maillard. All Rights Reserved.
// clang-format on

#include <exception>

#include "commands/Add.hpp"
#include "commands/Build.hpp"
#include "commands/Config.hpp"
#include "commands/Init.hpp"
#include "commands/Install.hpp"
#include "commands/List.hpp"
#include "commands/Login.hpp"
#include "commands/Logout.hpp"
#include "commands/Uninstall.hpp"
#include "commands/Update.hpp"
#include "commands/Use.hpp"
#include "framework/cli/App.hpp"
#include "framework/cli/CommandBase.hpp"
#include "framework/Exception.hpp"
#include "framework/Timer.hpp"
#include "framework/ui/UI.hpp"
#include "framework/util/terminal.hpp"

using ZCCli = zc::cli::RootCommand<
  "zc", "The C and C++ package manager and build system!", zc::app::Install, zc::app::Update,
  zc::app::Uninstall, zc::app::Use, zc::app::List, zc::app::Config, zc::app::Login, zc::app::Logout,
  zc::app::Init, zc::app::Build, zc::app::Add
>;

int main(const int argc, const char **argv)
{
  ZC_DEBUG_TIMER("Total time");

#ifdef ZC_DEBUG
  constexpr std::string_view version_str = "ZC v" ZC_VERSION " (debug)";
#else
  constexpr std::string_view version_str = "ZC v" ZC_VERSION;
#endif

  zc::cli::App<ZCCli> app(version_str);

  try {
    app.run(argc, argv);
    return 0;
  } catch (const zc::Exception &e) {
    zc::ui.printException(e);
    return e.code();
  } catch (const std::exception &e) {
    zc::ui.error("{}Unexpected fatal error:{} {}", zc::red, zc::reset, e.what());
    return 255;
  }
}
