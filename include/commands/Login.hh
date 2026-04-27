#include "commands/Command.hh"
#include "objects/Controllers/GlobalController.hh"

class Login : public Command
{
public:
  Login();
  int operator()() override;

private:
  GlobalController g_;
};
