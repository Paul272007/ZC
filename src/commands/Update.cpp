#include <filesystem>

#include "Update.h"
#include "commands/Command.h"
#include "helpers.h"
// #include "project/Project.h"

ZC_DEV_CONFIG_JSON

namespace zc {


Update::Update(bool force, const std::string &path, std::vector<std::string> &targets)
    : Command(force), path_(path), targets_(parse_targets(targets))
{
}

int Update::operator()()
{
  if (!path_.empty())
  {
    if (!targets_.empty())
      throw ZCException(
          ZCE_INCOMPATIBLE_FLAGS, "Cannot update from remote and from local project at the same time"
      );

    reg_.update_from_path(path_, force_);
    return 0;
  }
  if (targets_.empty())
  {
    // TODO : implement p.update_dependencies();
    // Project p;
    // p.update_dependencies();
    // return 0;
  }

  json index = Network::get().get_index();
  for (const auto &target : targets_) reg_.update_from_server(target.name, target.version, index, force_);
  return 0;
}

} // namespace zc
