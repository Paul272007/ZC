#pragma once

#include "commands/Languages/Languages.h"
#include "Context.h"

namespace zc
{

class LanguagesAdd : public Languages
{
public:
  LanguagesAdd(const LanguagesContext &ctx);

  void operator()() override;
};

} // namespace zc
