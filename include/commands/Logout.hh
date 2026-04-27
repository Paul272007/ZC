#include "commands/Command.hh"
#include "objects/Controllers/GlobalController.hh"

class Logout : public Command
{
public:
  Logout();
  int operator()() override;

private:
  GlobalController g_;
};
