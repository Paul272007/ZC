#pragma once

#include <filesystem>
#include <vector>

#include "config/Dependency.h"
#include "pkgs/Pkg.h"

namespace zc
{

std::vector<Dependency>
get_file_includes(const std::filesystem::path &file, const std::vector<RegistryPkg> &pkgs);

} // namespace zc
