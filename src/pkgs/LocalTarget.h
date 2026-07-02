#pragma once

#include <string>
#include <vector>

#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "pkgs/Registry.h"
#include "Version.h"

namespace zc
{

struct LocalTarget
{
  const std::string name;
  const Version     version;

  static std::vector<LocalTarget> parse(const std::vector<std::string> &strs)
  {
    std::vector<Target>      pairs = parse_targets(strs);
    std::vector<LocalTarget> targets;

    for (CAA[name, version] : pairs)
      if (version.is_empty() && !rg().is_installed(name))
        throw ZCException(ZCE_NOT_FOUND, "Package '" + name + "' is not installed.");
      else if (!rg().is_installed(name, version))
        throw ZCException(
          ZCE_NOT_FOUND, "Package '" + name + "' at version '" + version.string() + "' is not installed."
        );
      else
        targets.emplace_back(name, version);

    return targets;
  }
};

} // namespace zc
