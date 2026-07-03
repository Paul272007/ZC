#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "commands/Languages/Languages.h"

namespace zc
{

class LanguagesShow : public Languages
{
public:
  LanguagesShow(
    bool force, const std::filesystem::path &p_root, const std::vector<std::string> &languages, bool global
  );

  void operator()() override;
};

} // namespace zc
