#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "config/Dependency.h"
#include "pkgs/Pkg.h"

namespace zc
{

struct Declarations
{
  // Preprocessing
  std::vector<std::string> includes;
  std::vector<std::string> macros;
  // Types definitions
  std::vector<std::string> typedefs;
  std::vector<std::string> enums;
  std::vector<std::string> structs;
  std::vector<std::string> unions;
  // Rest
  std::vector<std::string> globals;
  std::vector<std::string> functions;
};

std::vector<Dependency>
get_file_includes(const std::filesystem::path &file, const std::map<std::string, Pkg> &pkgs);

Declarations parse_declarations(const std::filesystem::path &file);

} // namespace zc
