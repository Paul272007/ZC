#include "LanguagesShow.h"

#include "Context.h"

namespace zc
{

LanguagesShow::LanguagesShow(const LanguagesContext &ctx) : Languages(ctx) {}

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
