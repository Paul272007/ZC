/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include "../Version.h"

namespace zc
{

struct Dependency
{
  std::string name;
  bool static_link = false;
  Version version = {0, 0, 0};
};

} // namespace zc
