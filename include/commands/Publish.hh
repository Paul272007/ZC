#include <commands/Command.hh>
#include <objects/ProjectSettings.hh>

#define GH_REPO "Paul272007/ZC-Registry"

class Publish : public Command
{
public:
  Publish(const bool force, const bool quiet);
  int operator()() override;

private:
  ProjectSettings p_settings_;
};
