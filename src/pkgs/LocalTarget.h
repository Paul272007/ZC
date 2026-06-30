#pragma once

#include <string>
#include <vector>

#include "../Version.h"

namespace zc
{

struct LocalTarget
{
  const std::string name;
  const Version     version; // default = empty version

  static std::vector<LocalTarget> parse(const std::vector<std::string> &strs)
  {
    if (strs.empty())
      return {};
    std::vector<LocalTarget> targets;

    for (const auto &target : strs)
      if (const size_t at_pos = target.find('@'); at_pos != std::string::npos)
        targets.emplace_back(target.substr(0, at_pos), target.substr(at_pos + 1));
      else
        targets.emplace_back(target, Version{ 0, 0, 0 });

    return targets;
  }
};

} // namespace zc
