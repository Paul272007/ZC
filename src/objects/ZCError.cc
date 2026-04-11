#include <interface.hh>
#include <objects/ZCError.hh>

using namespace std;

ZCError::ZCError() : code_(SUCCESS), message_("Feature not implemented yet!")
{
}

ZCError::ZCError(const ErrorCode code, const string &message) : code_(code), message_(message)
{
}

void ZCError::display(ostream &stream) const
{
  stream << RED << "[ERROR]   " << COLOR_RESET;

#ifdef DEBUG_MODE
  stream << "(exit code: " << code_ << ") ";
#endif

  stream << message_;
}

ostream &operator<<(ostream &stream, const ZCError &error)
{
  error.display(stream);
  return stream;
}

int ZCError::getCode_() const
{
  return (int)code_;
}
