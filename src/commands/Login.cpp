#include "commands/Login.h"

namespace zc
{

Login::Login(const bool force) : Command(force)
{
}

int Login::operator()()
{
  gc_.login(); // TODO : force_ logs in even if an account is already signed in
  return 0;
}

} // namespace zc
