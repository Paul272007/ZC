#pragma once

#include "Context.h"
#include "helpers.h"

namespace zc
{

class BuildCommand
{
public:
  BuildCommand(const BuildCommand &)            = delete;
  BuildCommand(BuildCommand &&)                 = delete;
  BuildCommand &operator=(const BuildCommand &) = delete;
  BuildCommand &operator=(BuildCommand &&)      = delete;
  virtual ~BuildCommand()                       = default;

protected:
  const size_t jobs_;

  explicit BuildCommand(const BuildContext &ctx)
    : jobs_(ctx.jobs_given ? get_jobs_count(ctx.input_jobs) : 1)
  {
  }
};

} // namespace zc
