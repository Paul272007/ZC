#pragma once

#include "framework/util/types.hpp"

namespace zc::app
{

struct CommandCtx {
  bool force   = false;
  bool quiet   = false;
  bool verbose = false;

  stdfs::path in_path;
  stdfs::path out_path;
  stdfs::path err_path;

  /// @brief The root of the project currently worked on
  stdfs::path project_path = stdfs::current_path();
};

} // namespace zc::app
