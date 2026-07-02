#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "commands/ProjectCommand.h"

namespace zc
{

class Languages : public ProjectCommand
{
protected:
  Languages(
    bool force, const std::filesystem::path &p_root, const std::vector<std::string> &languages, bool global
  )
    : ProjectCommand(force, p_root, !global), global_(global)
  {
    for (const auto &str_l : languages)
      if (const Language l = language_from_str(str_l); l == UNKNOWN_LANGUAGE)
        throw ZCException(ZCE_UNSUPPORTED_LANGUAGE, "Unknown language: " + str_l);
      else
        languages_.push_back(l);
  }

  std::vector<Language> languages_;

  const bool global_;
};

} // namespace zc
