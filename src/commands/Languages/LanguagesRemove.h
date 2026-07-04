#pragma once

#include "commands/Languages/Languages.h"

namespace zc
{

class LanguagesRemove : public Languages
{
public:
  LanguagesRemove(const LanguagesContext &ctx);

  void operator()() override;
};

} // namespace zc
