#pragma once

#include <filesystem>
#include <map>

#include "config/Dependency.h"
#include "pkgs/Pkg.h"

namespace zc
{

std::vector<Dependency>
get_file_includes(const std::filesystem::path &file, const std::map<std::string, Pkg> &pkgs);

} // namespace zc
