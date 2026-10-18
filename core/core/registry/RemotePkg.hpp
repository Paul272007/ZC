#pragma once

#include <string>

#include "framework/Version.hpp"

namespace zc::core
{

struct RemotePkg {
  std::string name;
  std::string url;
  std::string sha256;
  Version     version;
};

} // namespace zc::core
