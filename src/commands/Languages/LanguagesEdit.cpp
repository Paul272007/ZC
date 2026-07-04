#include "LanguagesEdit.h"

namespace zc
{

LanguagesEdit::LanguagesEdit(const LanguagesContext &ctx) : Languages(ctx) {}

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
