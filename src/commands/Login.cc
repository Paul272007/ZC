#include "commands/Login.hh"
#include "commands/Command.hh"

Login::Login() : Command(false, false), g_(logger_, false)
{
}

int Login::operator()()
{
  g_.login();
  return 0;
}
