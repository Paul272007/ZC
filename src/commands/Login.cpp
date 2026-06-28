#include "commands/Login.h"

namespace zc
{

Login::Login(const bool force) : Command(force) {}

void Login::operator()()
{
  gc_.login(force_);
}

} // namespace zc
