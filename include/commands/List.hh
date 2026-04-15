#pragma once

#include <commands/Command.hh>
#include <objects/Registry.hh>

class List : public Command
{
public:
  List(bool force, bool quiet);

  int operator()() override;

private:
  const Registry registry_;
};
