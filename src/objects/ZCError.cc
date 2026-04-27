#include <interface.hh>
#include <objects/ZCError.hh>

using namespace std;

ZCError::ZCError(const ReturnCode code, const string &message) : code_(code), message_(message)
{
}

ostream &operator<<(ostream &stream, const ZCError &error)
{
  stream << "[" RED "ERROR" COLOR_RESET "]   ";

#ifdef DEBUG_MODE
  stream << "(exit code: " << error.code_ << ") ";
#endif

  stream << error.message_;
  return stream;
}

int ZCError::code() const
{
  return (int)code_;
}

const char *ZCError::what() const noexcept
{
  return message_.c_str();
}
