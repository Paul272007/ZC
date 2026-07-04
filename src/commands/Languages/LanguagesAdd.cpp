#include "LanguagesAdd.h"

#include "Context.h"

namespace zc
{

LanguagesAdd::LanguagesAdd(const LanguagesContext &ctx) : Languages(ctx) {}

void LanguagesAdd::operator()()
{
  if (global_)
    ; // TODO: gc().add_language(l);
  else
    for (const auto &l : languages_)
      p().pconf.add_language(l);
}

} // namespace zc
