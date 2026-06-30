#pragma once

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

#include "../excepts/ExitCode.h"
#include "../excepts/ZCException.h"
#include "../helpers.h"
#include "../Version.h"
#include "Network.h"

namespace zc
{

struct RemoteTarget
{
  const std::string name;
  const std::string url;
  const std::string sha256;
  const Version     version; // default = empty version

  static std::vector<RemoteTarget> parse(const std::vector<std::string> &targets)
  {
    if (targets.empty())
      return {};
    std::vector<std::pair<std::string, Version>> pairs;

    for (const auto &target : targets)
      if (const size_t at_pos = target.find('@'); at_pos != std::string::npos)
        pairs.emplace_back(target.substr(0, at_pos), target.substr(at_pos + 1));
      else
        pairs.emplace_back(target, Version{ 0, 0, 0 });

    return get_targets(pairs);
  }

  static std::vector<RemoteTarget> get_targets(std::vector<std::pair<std::string, Version>> &pairs)
  {
    if (pairs.empty())
      return {};
    std::vector<RemoteTarget> results;
    const nlohmann::json      index = net().get_index();
    nlohmann::json            pkgs;

    get_key(index, "packages", pkgs);

    for (auto &[name, requested_version] : pairs)
    {
      std::string version;
      std::string url;
      std::string sha256;

      if (!pkgs.contains(name))
        throw ZCException(ZCE_PKG_NOT_FOUND, "Package '" + name + "' not found.");

      const nlohmann::json pkg = pkgs[name];
      if (!pkg.is_object())
        throw ZCException(ZCE_TYPE_ERROR, "Incorrect package declaration in index.");

      version = get_version(pkg, requested_version.string());
      url     = get_url(pkg, version);
      sha256  = get_sha256(pkg, version);

      results.push_back({ .name = name, .url = url, .sha256 = sha256, .version = version });
    }
    return results;
  }

  static std::string get_version(const nlohmann::json &pkg, const std::string &requested_version)
  {
    std::string version;

    if (requested_version.empty())               // If no requested version take latest
      version = pkg["latest"].get<std::string>();
    else if (!pkg["versions"].contains(version)) // If a version was requested verify that is exists
      throw ZCException(ZCE_NOT_FOUND, "Version " + requested_version + " not found.");
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
