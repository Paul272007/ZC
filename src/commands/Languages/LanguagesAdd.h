#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "commands/Languages/Languages.h"

namespace zc
{

class LanguagesAdd : public Languages
{
public:
  LanguagesAdd(
    bool force, const std::filesystem::path &p_root, const std::vector<std::string> &languages, bool global
  );

  void operator()() override;
};

} // namespace zc
