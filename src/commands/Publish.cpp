#include "commands/Publish.h"

#include "commands/ProjectCommand.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Publish::Publish(const bool force, const std::filesystem::path &p_root) : ProjectCommand(force, p_root) {}

void Publish::operator()()
{
  p().publish();
}

} // namespace zc
