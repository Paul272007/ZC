#pragma once

#include "commands/Languages/Languages.h"
#include "Context.h"

namespace zc
{

class LanguagesShow : public Languages
{
public:
  LanguagesShow(const LanguagesContext &ctx);

  void operator()() override;
};

} // namespace zc
