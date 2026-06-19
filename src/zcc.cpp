// ZCC: Version 0.1.1
//
//  ________  ________  ________
// |\_____  \|\   ____\|\   ____\
//  \|___/  /\ \  \___|\ \  \___|
//      /  / /\ \  \    \ \  \
//     /  /_/__\ \  \____\ \  \____
//    |\________\ \_______\ \_______\
//     \|_______|\|_______|\|_______|
//
// Copyright (c) 2026 Paul Maillard. All Rights Reserved.

#include <string>

#include "CLI11.h"
#include "ZCCompiler.h"
#include "helpers.h"
#include "ui/Interface.h"
#include "ui/ui_utils.h"

using namespace zc;

ZC_DEV_CONFIG

int main(const int argc, char *argv[])
{
  bool quiet = false;
  bool link = false;
  string compiler;
  vector<string> sources;
  vector<string> flags;

  // --- Initialize app
  CLI::App app("ZCC utility for C and C++");
  argv = app.ensure_utf8(argv);
  app.set_version_flag("--version,-v", "ZCC v0.1.1 (unstable)");

  app.add_option("--sources,-S", sources, "Sources to compile")->required();
  app.add_option("--compiler,-C", compiler, "Compiler to use")->required();
  app.add_option("--flags,-F", flags, "Flags to add")->required();

  app.add_flag("--link,-L", link, "Link binary");
  app.add_flag("--quiet,-q", quiet, "Do not show any messages");

  try
  {
    app.parse(argc, argv);
    Interface::get(quiet); // create Interface instance before any other command
    ZCCompiler zcc(compiler, flags, sources);
    zcc();
  }
  catch (const CLI::ParseError &e)
  {
    return app.exit(e);
  }
  catch (const ZCException &e)
  {
    cerr << e << endl;
    return e.code();
  }
  catch (const exception &e)
  {
    cerr << RED << "Unexpected error: " << RESET << e.what() << endl;
    return -1;
  }
}
