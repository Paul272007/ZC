/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _ZCEXCEPTION_H
#define _ZCEXCEPTION_H

#include <exception>
#include <ostream>
#include <string>

#include "ExitCode.h"

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

#endif //_ZCEXCEPTION_H
