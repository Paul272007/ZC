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
  explicit ZCException(ExitCode code = ZCE_SUCCESS, std::string message = "Feature not implemented");

  [[nodiscard]] int code() const;
  [[nodiscard]] const char *what() const noexcept override;

  friend std::ostream &operator<<(std::ostream &stream, const ZCException &zc_exception);

private:
  const ExitCode    code_;
  const std::string message_;
};

} // namespace zc
