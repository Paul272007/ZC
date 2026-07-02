#include "LanguagesEdit.h"

namespace zc
{

LanguagesEdit::LanguagesEdit(
  const bool force, const std::filesystem::path &p_root, const std::vector<std::string> &languages,
  const bool global
)
  : Languages(force, p_root, languages, global)
{
}

void LanguagesEdit::operator()()
{
  if (global_)
    ; // TODO: gc().add_language(l);
  else if (languages_.empty())
    p().pconf.edit_languages();
  else
    for (const auto &l : languages_)
      p().pconf.edit_language(l);
}

} // namespace zc
