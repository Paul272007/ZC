#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace zc
{

struct CommandContext
{
  bool                  force  = false;
  std::filesystem::path p_root = "";
};

struct BuildContext
{
  bool jobs_given = false;
  int  input_jobs = 1;
};

struct LanguagesContext
{
  bool                     global = false;
  CommandContext           c_ctx;
  std::vector<std::string> languages;
};

struct InstallContext
{
  bool                     sync = false;
  std::filesystem::path    path;
  std::vector<std::string> targets;
};

} // namespace zc
