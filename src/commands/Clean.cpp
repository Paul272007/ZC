#include "commands/Clean.h"

#include "commands/ProjectCommand.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Clean::Clean(const bool force, const std::filesystem::path &p_root) : ProjectCommand(force, p_root) {}

void Clean::operator()()
{
  p().clean();
}

} // namespace zc
