#include "LanguagesRemove.h"

namespace zc
{

LanguagesRemove::LanguagesRemove(
  const bool force, const std::filesystem::path &p_root, const std::vector<std::string> &languages,
  const bool global
)
  : Languages(force, p_root, languages, global)
{
}

void LanguagesRemove::operator()()
{
  if (global_)
    ; // TODO: gc().remove_language(l);
  else
    for (const auto &l : languages_)
      p().pconf.remove_language(l);
}

} // namespace zc
