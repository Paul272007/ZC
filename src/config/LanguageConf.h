#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "../Language.h"
#include "../helpers.h"

namespace zc
{

struct LanguageConf
{
  Language name;
  std::string std;
  std::string compiler;
  std::vector<std::string> flags;
};

inline void to_json(nlohmann::json &j, const LanguageConf &p)
{
  j = nlohmann::json{{"compiler", p.compiler}, {"std", p.std}, {"flags", p.flags}};
}

inline void from_json(const nlohmann::json &j, LanguageConf &p)
{
  get_key(j, "compiler", p.compiler);
  get_key(j, "std", p.std);
  get_key(j, "flags", p.flags);
}

} // namespace zc
