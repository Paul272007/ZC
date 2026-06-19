//
// Created by paul on 15/06/2026.
//

#include "commands/Publish.h"
#include "commands/Command.h"
#include "helpers.h"
#include "project/Project.h"

ZC_DEV_CONFIG

namespace zc
{

Publish::Publish(const bool force, const std::filesystem::path &p_root)
    : Command(force), p_root_(get_project_root(p_root))
{
}

void Publish::operator()()
{
  Project p(p_root_);
  p.publish();
}

} // namespace zc
