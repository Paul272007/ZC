#pragma once

#include <string>
#include <vector>

#include "core/global-config/GlobalConfig.hpp"
#include "core/util/helpers.hpp"
#include "core/util/json_helpers.hpp"
#include "Language.hpp"

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
  // TODO: change to use global config
  get_key_with_default(j, "compiler", c.compiler, default_compiler(c.language));
  get_key_with_default(j, "std", c.std, default_std(c.language));
  get_key_with_default(j, "flags", c.flags, default_flags(c.language));
}

} // namespace zc::core
