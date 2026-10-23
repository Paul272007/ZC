#pragma once

#include <string>
#include <vector>

#include "core/project/Language.hpp"
#include "core/util/json_helpers.hpp"

namespace zc::core
{

struct LanguageConf {
  Language    language;
  std::string std;
  std::string compiler;

  std::vector<std::string> flags;

  bool operator==(const LanguageConf &l) const = default;
};

inline void to_json(json &j, const LanguageConf &c)
{
  j = json{
    { "compiler", c.compiler },
    { "std", c.std },
    { "flags", c.flags },
  };
}

inline void from_json(const json &j, LanguageConf &c)
{
  get_optional_key(j, "compiler", c.compiler);
  get_optional_key(j, "std", c.std);
  get_optional_key(j, "flags", c.flags);
}

} // namespace zc::core
