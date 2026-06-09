/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _DEPENDENCY_H
#define _DEPENDENCY_H

#include "../Version.h"

struct Dependency
{
  std::string name;
  bool static_link = false;
  Version version;
};

#endif //_DEPENDENCY_H
