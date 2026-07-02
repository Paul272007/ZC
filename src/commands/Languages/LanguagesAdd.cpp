#include "LanguagesAdd.h"

namespace zc
{

LanguagesAdd::LanguagesAdd(
  const bool force, const std::filesystem::path &p_root, const std::vector<std::string> &languages,
  const bool global
)
  : Languages(force, p_root, languages, global)
{
}

void LanguagesAdd::operator()()
{
  if (global_)
    ; // TODO: gc().add_language(l);
  else
    for (const auto &l : languages_)
      p().pconf.add_language(l);
}

} // namespace zc
