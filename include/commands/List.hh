#pragma once

#include <commands/Command.hh>
#include <objects/Registry.hh>

class List : public Command
{
public:
  List(bool force, bool quiet, bool std, bool all);

  int execute() override;

private:
  const bool std_;
  const bool all_;
  const Registry registry_;
};
