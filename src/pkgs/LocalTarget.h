#pragma once

#include <string>

#include "helpers.h"
#include "pkgs/Pkg.h"
#include "pkgs/Registry.h"
#include "Version.h"

namespace zc
{

/**
 * @class LocalTarget
 * @brief We know that the pkg and the version exist
 */
struct LocalTarget
{
private:
  LocalTarget(std::string n, std::string o, const Version &v)
    : name(std::move(n)), origin(std::move(o)), version(v)
  {
  }

public:
  const std::string name;
  const std::string origin;
  const Version     version;

  [[nodiscard]] std::string string() const
  {
    return std::format("{}:{}@{}", origin, name, version.string());
  }

  static LocalTarget get_target(const Target &t)
  {
    if (t.second.is_empty() || t.second.is_default())
    {
      const Pkg &p = rg().get_pkg(t.first); // <- checks that pkg exists
      if (!p.versions.contains(p.default_version))
        throw ZCException(
          ZCE_PKG_NOT_FOUND, "Default version '" + p.default_version.string() + "' for package '" + p.name +
                               "' does not exist."
        );
      return { p.name, p.origin, p.default_version };
    }
    if (t.second.is_latest())
    {
      const Pkg &p = rg().get_pkg(t.first);                  // <- checks that pkg exists
      return { t.first, p.origin, rg().get_latest(p.name) }; // <- latest version always exists
    }
    const Pkg &p = rg().get_pkg(t.first);
    if (!p.versions.contains(t.second))
      throw ZCException(
        ZCE_PKG_NOT_FOUND,
        "Version '" + t.second.string() + "' for package '" + p.name + "' does not exist."
      );
    return { p.name, p.origin, t.second };
  }
};

} // namespace zc
