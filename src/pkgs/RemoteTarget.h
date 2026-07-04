#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"
#include "helpers.h"
#include "pkgs/Network.h"
#include "pkgs/Registry.h"
#include "Version.h"

namespace zc
{

struct RemoteTarget
{
  const std::string name;
  const std::string url;
  const std::string sha256;
  const Version     version; // default = empty version

  static RemoteTarget get_target(const Target &t)
  {
    static const nlohmann::json pkgs = []
    {
      const nlohmann::json &index = net().get_index();
      nlohmann::json        index_pkgs;
      get_key(index, "packages", index_pkgs);
      return index_pkgs;
    }();
    std::string version;
    std::string url;
    std::string sha256;

    if (!pkgs.contains(t.first))
      throw ZCException(ZCE_PKG_NOT_FOUND, "Package '" + t.first + "' not found.");

    const nlohmann::json &pkg = pkgs[t.first];
    if (!pkg.is_object())
      throw ZCException(ZCE_TYPE_ERROR, "Incorrect package declaration in index.");

    version = get_version(pkg, t.second).string();
    url     = get_url(pkg, version);
    sha256  = get_sha256(pkg, version);

    return { .name = t.first, .url = url, .sha256 = sha256, .version = version };
  }

  static Version get_version(const nlohmann::json &pkg, const Version &requested_version)
  {
    Version version;

    if (requested_version.is_empty() || requested_version.is_latest() || requested_version.is_default())
      version = pkg["latest"].get<std::string>();
    else if (!pkg["versions"].contains(version)) // If a version was requested verify that is exists
      throw ZCException(ZCE_NOT_FOUND, "Version " + requested_version.string() + " not found.");
    else
      version = requested_version;

    return version;
  }

  static std::string get_url(const nlohmann::json &pkg, const std::string &version)
  {
    return pkg["versions"][version].value("url", "");
  }

  static std::string get_sha256(const nlohmann::json &pkg, const std::string &version)
  {
    return pkg["versions"][version].value("sha256", "SKIP");
  }
};

} // namespace zc
