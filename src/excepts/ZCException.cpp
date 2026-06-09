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

ZCException::ZCException()
{
}

/**
 * @param code
 * @param message
 */
ZCException::ZCException(ExitCode code, const std::string &message)
{
}

/**
 * @return const char*
 */
const char *ZCException::what() const noexcept
{
  return "";
}

/**
 * @return int
 */
int ZCException::code()
{
  return 0;
}
