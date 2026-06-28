#pragma once

#include "../helpers.h"
#include "../Version.h"

namespace zc
{

struct Dependency
{
  std::string name;
  std::string origin = "main"; // forbidden to change dependency origin

  bool    static_link = false;
  Version version;

  bool operator==(const Dependency &) const = default;
};

inline void to_json(nlohmann::json &j, const Dependency &d)
{
  j = nlohmann::json{
    { "origin", d.origin },
    { "static", d.static_link },
    { "version", d.version },
  };
}

inline void from_json(const nlohmann::json &j, Dependency &d)
{
  // Name is handled elsewhere
  get_key(j, "origin", d.origin);
  get_key(j, "static", d.static_link, d.static_link);
  get_key(j, "version", d.version);
}

} // namespace zc
