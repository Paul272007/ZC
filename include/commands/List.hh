#pragma once

#include <commands/Command.hh>
#include <objects/Registry.hh>

class List : public Command
{
public:
  List(bool force, bool quiet, bool global, bool templates, bool p_templates);

  int operator()() override;

private:
  const Registry registry_;
  const bool templates_;
  const bool p_templates_;
};
