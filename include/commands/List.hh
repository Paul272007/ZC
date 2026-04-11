#pragma once

#include <commands/Command.hh>
#include <objects/Registry.hh>

class List : public Command
{
public:
  List(const bool force, const bool quiet, const bool std, const bool all);

  int execute() override;

private:
  const bool std_;
  const bool all_;
  const Registry &registry_;
};
