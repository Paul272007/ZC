#pragma once

#include <map>

#include "framework/Version.hpp"
#include "util/ListType.hpp"

namespace zc::app
{

std::map<std::string, Version, std::less<>> parse_targets(std::vector<std::string> &targets);

ListType parse_type(std::string_view type);

} // namespace zc::app
