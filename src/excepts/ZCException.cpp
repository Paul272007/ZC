/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include "ZCException.h"
#include "../helpers.h"

ZC_DEV_CONFIG

/**
 * ZCException implementation
 */

/**
 * @param code
 * @param message
 */
ZCException::ZCException(ExitCode code, const std::string &message) : code_(code), message_(message)
{
}

/**
 * @return const char*
 */
const char *ZCException::what() const noexcept
{
  return message_.c_str();
}

/**
 * @return int
 */
int ZCException::code()
{
  return (int)code_;
}
