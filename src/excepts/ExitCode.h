

/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

namespace zc
{

enum ExitCode
{
  ZCE_SUCCESS = 0,
  // File exceptions
  ZCE_NOT_FOUND,
  ZCE_WRITING_ERROR,
  ZCE_READING_ERROR,
  ZCE_PARSING_ERROR,
  // Configuration exceptions
  ZCE_CONTENT_ERROR,
  ZCE_MISSING_PROPERTY,
  ZCE_UNSUPPORTED_LANGUAGE,
  ZCE_TYPE_ERROR,
  // CLI exceptions
  ZCE_BAD_COMMAND,
  ZCE_INCOMPATIBLE_FLAGS,
  ZCE_ABORTED,
  // Internal exceptions
  ZCE_NETWORK_ERROR,
  ZCE_INTERNAL_ERROR,
  ZCE_GIT_ERROR,
  ZCE_ARCHIVE_ERROR,
  // Registry/packages exceptions
  ZCE_PKG_NOT_FOUND,
  ZCE_ALREADY_INSTALLED,
  ZCE_ORIGIN_MISMATCH,
  ZCE_VERSION_ALREADY_EXISTS,
  ZCE_HASH_MISMATCH,
  ZCE_RECURSIVE_DEPENDENCY,
  // Project build/execution exceptions
  ZCE_COMPILATION_ERROR,
  ZCE_RUNTIME_ERROR,
  ZCE_NO_SOURCE_FILES,
  ZCE_NOT_A_ZC_PROJECT,
  // Publishing exceptions
  ZCE_AUTHENTICATION_ERROR,
  ZCE_MISSING_TOKEN,
  ZCE_LOCAL_DEPENDENCY,
};

} // namespace zc
