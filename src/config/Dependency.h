#pragma once

#include "../Version.h"

namespace zc
{

struct Dependency
{
  std::string name;
  std::string origin      = "main";
  bool        static_link = false;
  Version     version;

  bool operator==(const Dependency &) const = default;
};

} // namespace zc
