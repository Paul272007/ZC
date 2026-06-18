#include "Uninstall.h"
#include "commands/Command.h"
#include "helpers.h"

ZC_DEV_CONFIG

namespace zc
{

Uninstall::Uninstall(bool force, std::vector<std::string> &targets) : Command(force), targets_(targets)
{
}

int Uninstall::operator()()
{
  for (const auto &target : targets_) reg_.uninstall(target);
  return 0;
}

} // namespace zc
