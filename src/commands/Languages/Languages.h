#pragma once

#include "commands/Command.h"
#include "Context.h"

namespace zc
{

class Languages : public Command
{
protected:
  Languages(const LanguagesContext &ctx) : Command(ctx.c_ctx, !ctx.global), global_(ctx.global)
  {
    for (const auto &str_l : ctx.languages)
      if (const Language l = language_from_str(str_l); l == UNKNOWN_LANGUAGE)
        throw ZCException(ZCE_UNSUPPORTED_LANGUAGE, "Unknown language: " + str_l);
      else
        languages_.push_back(l);
  }

  std::vector<Language> languages_;

  const bool global_;
};

} // namespace zc
