#pragma once

#include "framework/util/types.hpp"

namespace zc::app
{

enum class ListType : u8 {
  Packages,
  Dependencies,
  FileTemplates,
  ProjectTemplates,
  Remote,
  Unknown,
};

} // namespace zc::app
