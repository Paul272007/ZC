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
  ZCException();

  /**
   * @param code
   * @param message
   */
  ZCException(ExitCode code, const std::string &message);

  virtual const char *what() const noexcept override;

  int code();

private:
  const std::string message_ = "Feature not implemented";
  const ExitCode code_ = ZCE_SUCCESS;
};

#endif //_ZCEXCEPTION_H
