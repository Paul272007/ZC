#pragma once

#include <vector>

namespace zc::core
{

struct ProjectTargetResolvedFlags {
  std::vector<std::string> macros;      // -D
  std::vector<std::string> includes;    // -I
  std::vector<std::string> static_libs; // paths to .a
  std::vector<std::string> ldflags;     // -L, -l
};

} // namespace zc::core
