#pragma once

#include <cstdint>

namespace zc
{

enum ExitCode : std::uint8_t
{
  ZCE_SUCCESS = 0,
  // File exceptions
  ZCE_NOT_FOUND     = 1,
  ZCE_WRITING_ERROR = 2,
  ZCE_READING_ERROR = 3,
  ZCE_PARSING_ERROR = 4,
  // Configuration exceptions
  ZCE_CONTENT_ERROR        = 10,
  ZCE_MISSING_PROPERTY     = 11,
  ZCE_UNSUPPORTED_LANGUAGE = 12,
  ZCE_TYPE_ERROR           = 13,
  ZCE_BAD_STRUCTURE        = 14,
  // CLI exceptions
  ZCE_BAD_COMMAND        = 20,
  ZCE_INCOMPATIBLE_FLAGS = 21,
  ZCE_ABORTED            = 22,
  // Internal exceptions
  ZCE_INTERNAL_ERROR = 30,
  ZCE_NETWORK_ERROR  = 31,
  ZCE_GIT_ERROR      = 32,
  ZCE_ARCHIVE_ERROR  = 33,
  // Registry/packages exceptions
  ZCE_PKG_NOT_FOUND          = 40,
  ZCE_ALREADY_INSTALLED      = 41,
  ZCE_ORIGIN_MISMATCH        = 42,
  ZCE_VERSION_ALREADY_EXISTS = 43,
  ZCE_HASH_MISMATCH          = 44,
  ZCE_RECURSIVE_DEPENDENCY   = 45,
  // Project build/execution exceptions
  ZCE_COMPILATION_ERROR = 50,
  ZCE_RUNTIME_ERROR     = 51,
  ZCE_NO_SOURCE_FILES   = 52,
  ZCE_NOT_A_ZC_PROJECT  = 53,
  // Publishing exceptions
  ZCE_AUTHENTICATION_ERROR = 60,
  ZCE_MISSING_TOKEN        = 61,
  ZCE_LOCAL_DEPENDENCY     = 62,
};

} // namespace zc
