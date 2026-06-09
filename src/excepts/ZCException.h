/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _ZCEXCEPTION_H
#define _ZCEXCEPTION_H

#include <exception>
#include <string>

#include "ExitCode.h"

class ZCException : public std::exception
{
public:
  /**
   * @param code
   * @param message
   */
  ZCException(ExitCode code = ZCE_SUCCESS, const std::string &message = "Feature not implemented");

  virtual const char *what() const noexcept override;

  int code();

private:
  const std::string message_;
  const ExitCode code_;
};

#endif //_ZCEXCEPTION_H
