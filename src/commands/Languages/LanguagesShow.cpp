#include "LanguagesShow.h"

namespace zc
{

LanguagesShow::LanguagesShow(
  const bool force, const std::filesystem::path &p_root, const std::vector<std::string> &languages,
  const bool global
)
  : Languages(force, p_root, languages, global)
{
}

void LanguagesShow::operator()()
{
  if (global_)
    ; // TODO: gc().remove_language(l);

  else if (languages_.empty())
    p().pconf.show_languages();

  else
    for (const auto &l : languages_)
      p().pconf.show_language(l);
}

} // namespace zc
