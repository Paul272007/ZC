#pragma once
#include "context/BuildCommandCtx.hpp"
#include "framework/util/types.hpp"

namespace zc::app
{

class BuildCommand
{
protected:
  u32 jobs_;

  explicit BuildCommand(const BuildCommandCtx ctx) : jobs_(ctx.jobs) {}

public:
  BuildCommand(const BuildCommand &)            = delete;
  BuildCommand(BuildCommand &&)                 = delete;
  BuildCommand &operator=(const BuildCommand &) = delete;
  BuildCommand &operator=(BuildCommand &&)      = delete;
  virtual ~BuildCommand()                       = default;
};

} // namespace zc::app
