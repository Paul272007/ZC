#pragma once

#include "framework/util/types.hpp"

namespace zc::core
{

enum class ZCError : u8 {
  // Project errors
  NotInAProject,
  NoSources,
  TargetNotFound,
  BuildFailed,
  // Registry errors
  PkgNotFound,
  VersionNotFound,
  OriginMismatch,
  BrokenDependency,
  SymlinkAlreadyExists,
  // Remote index errors
  MissingUrl,
  MissingSha256,
  InvalidIndex,
  HashMismatch,
  // Dependency errors
  // CircularDependency,
  // InvalidDependency,
  // Config errors
  InvalidValue,
  TypeError,
  MissingKey,
  // External errors
  ArchiveError,
  ApiError,
  AuthenticationError,
  Unknown,
};

constexpr u8 exit_code_base(ZCError /**/) noexcept
{
  return 50;
}

} // namespace zc::core
