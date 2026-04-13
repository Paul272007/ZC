#pragma once

#include <commands/Command.hh>
#include <objects/Registry.hh>
#include <string>
#include <vector>

class Add : public Command
{
public:
  Add(const std::vector<std::string> &targets, const bool force, const bool quiet);

  int operator()() override;

private:
  const std::vector<std::string> targets_;
  Registry global_;
  Registry local_;
};
