#pragma once

#include "commands/Languages/Languages.h"
#include "Context.h"

namespace zc
{

class LanguagesEdit : public Languages
{
public:
  LanguagesEdit(const LanguagesContext &ctx);

  void operator()() override;
};

} // namespace zc
