/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _PKG_H
#define _PKG_H

#include <string>

#include "../Version.h"
#include "PkgType.h"

struct Pkg
{
  std::string name;
  std::string target;
  std::string origin = "main";
  PkgType type = UNDEF;
  Version version = {0, 0, 0};
};

#endif //_PKG_H
