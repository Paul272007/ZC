#pragma once

#include "framework/util/types.hpp"

namespace zc::core
{

enum class BuildMode : u8 {
  Auto,
  Debug,
  Release,
  ReleaseWithDebugInfo,
};

} // namespace zc::core
