#include "commands/Login.h"

namespace zc
{

Login::Login(const bool force) : Command(force) {}

void Login::operator()()
{
  gc_.login(); // TODO : force_ logs in even if an account is already signed in
}

} // namespace zc
