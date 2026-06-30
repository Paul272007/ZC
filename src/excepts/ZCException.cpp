#include "ZCException.h"

#include "../helpers.h"
#include "../ui/ui_utils.h"

ZC_DEV_CONFIG

namespace zc
{

ZCException::ZCException(const ExitCode code, std::string message)
  : code_(code), message_(std::move(message))
{
}

const char *ZCException::what() const noexcept
{
  return message_.c_str();
}

int ZCException::code() const
{
  return code_;
}

std::ostream &operator<<(std::ostream &stream, const ZCException &zc_exception)
{
  stream << RED "✗ Error: " RESET;

#ifdef DEBUG_MODE
  stream << "(exit code: " << static_cast<int>(zc_exception.code_) << ") ";
#endif

  stream << zc_exception.message_;
  return stream;
}

} // namespace zc
