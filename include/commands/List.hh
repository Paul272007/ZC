#pragma once

#include <commands/Command.hh>
#include <objects/ProjectsRegistry.hh>

class List : public Command
{
public:
  List(const bool force, const bool quiet);

  int execute() override;

private:
  const ProjectsRegistry &p_registry_;
};