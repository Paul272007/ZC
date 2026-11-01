#pragma once

#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "framework/Version.hpp"
#include "util/ListType.hpp"

namespace zc::app
{

struct DepSpec {
  std::string_view package;
  Version          version;
  std::string_view target;
};

DepSpec parse_dep_spec(std::string_view raw);

std::map<std::string, Version, std::less<>> parse_targets(std::vector<std::string> &targets);

ListType parse_list_type(std::string_view type);

} // namespace zc::app
