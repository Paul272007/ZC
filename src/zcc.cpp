// ZC: Version 0.1.1
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

#include "CLI11.h"
#include "helpers.h"

using namespace zc;

ZC_DEV_CONFIG

int main(const int argc, char *argv[])
{
  // --- Initialize app
  CLI::App app("ZCC utility for C and C++");
  argv = app.ensure_utf8(argv);
  app.set_version_flag("--version,-v", "ZCC v0.1.1 (unstable)");

  return 0;
}
