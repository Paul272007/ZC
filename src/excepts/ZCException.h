/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <exception>
#include <ostream>
#include <string>

#include "ExitCode.h"

namespace zc
{

class ZCException : public std::exception
{
public:
  /**
   * @param code
   * @param message
   */
  explicit ZCException(ExitCode code = ZCE_SUCCESS, const std::string &message = "Feature not implemented");

  const char *what() const noexcept override;

  int code() const;

  friend std::ostream &operator<<(std::ostream &stream, const ZCException &zc_exception);

private:
  const std::string message_;
  const ExitCode code_;
};

} // namespace zc
