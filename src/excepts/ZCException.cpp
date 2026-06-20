/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include "ZCException.h"
#include "../helpers.h"
#include "../ui/ui_utils.h"

ZC_DEV_CONFIG

namespace zc
{

/**
 * ZCException implementation
 */

/**
 * @param code
 * @param message
 */
ZCException::ZCException(const ExitCode code, const std::string &message) : message_(message), code_(code)
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
int ZCException::code() const
{
  return code_;
}

std::ostream &operator<<(std::ostream &stream, const ZCException &zc_exception)
{
  stream << RED "✗ Error: " RESET;

#ifdef DEBUG_MODE
  stream << "(exit code: " << zc_exception.code_ << ") ";
#endif

  stream << zc_exception.message_;
  return stream;
}

} // namespace zc
