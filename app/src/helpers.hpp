#pragma once

#include <map>

#include "framework/Version.hpp"

namespace zc::app
{

std::map<std::string, Version, std::less<>> parse_targets(std::vector<std::string> &targets);

} // namespace zc::app
