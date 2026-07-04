#include "LanguagesRemove.h"

#include "Context.h"

namespace zc
{

LanguagesRemove::LanguagesRemove(const LanguagesContext &ctx) : Languages(ctx) {}

void LanguagesRemove::operator()()
{
  if (global_)
    ; // TODO: gc().remove_language(l);
  else
    for (const auto &l : languages_)
      p().pconf.remove_language(l);
}

} // namespace zc
