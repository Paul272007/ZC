#pragma once

#include <exception>
#include <ostream>
#include <string>

enum ErrorCode
{
  // No errors
  ZC_SUCCESS = 0,
  // Program errors
  ZC_COMPILATION_ERROR = 10,
  ZC_EXECUTION_ERROR = 11,
  // File errors
  ZC_NOT_FOUND = 20,
  ZC_WRITING_ERROR = 21,
  ZC_READING_ERROR = 22,
  ZC_PARSING_ERROR = 23,
  // Configuration errors
  ZC_CONFIG_PARSING_ERROR = 30,
  ZC_CONFIG_NOT_FOUND = 31,
  ZC_CONFIG_READING_ERROR = 32,
  ZC_CONFIG_WRITING_ERROR = 33,
  ZC_CONFIG_CONTENT_ERROR = 34,
  ZC_CONFIG_MISSING_PROPERTY = 35,
  ZC_CONFIG_TYPE_ERROR = 36,
  // Command errors
  ZC_BAD_COMMAND = 40,
  ZC_UNSUPPORTED_LANGUAGE = 41,
  ZC_INCOMPATIBLE_FLAGS = 42,
  // Internal errors
  ZC_INTERNAL_ERROR = 60,
  // Package errors
  ZC_PACKAGE_NOT_FOUND = 70,
  // Build errors
  ZC_NO_SOURCE_FILES = 80,
  ZC_NOT_A_ZC_PROJECT = 81,
  // External errors
  ZC_GIT_ERROR = 90,
  ZC_CMAKE_ERROR = 91,
  // Project errors
  ZC_PROJECT_NOT_FOUND = 101,
  // User errors
  ZC_KEYBOARD_INTERRUPT = 130,
  ZC_OPERATIONS_ABORTED = 131,
};

class ZCError : public std::exception
{
public:
  /**
   * @brief Default constructor just to not get errors when throwing empty
   * errors
   */
  ZCError() = default;

  /**
   * @brief Create ZCError instance
   *
   * @param code The code corresponding to the error type
   * @param message A message that explains the error
   */
  ZCError(ErrorCode code, const std::string &message);

  /**
   * @brief << overload for ZCError
   *
   * @param stream The stream in which the error is written
   * @param error The error to be displayed
   */
  friend std::ostream &operator<<(std::ostream &stream, const ZCError &error);

  /**
   * @brief Get the error code as an int
   *
   * @return code_ as an int
   */
  int getCode_() const;

private:
  ErrorCode code_ = ZC_SUCCESS;
  std::string message_ = "Feature not implemented";
};
