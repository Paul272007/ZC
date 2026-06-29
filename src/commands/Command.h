#pragma once

#include "../config/GConf.h"
#include "../ui/Interface.h"

namespace zc
{

class Command
{
public:
  Command(const Command &)            = delete;
  Command &operator=(const Command &) = delete;
  Command(Command &&)                 = delete;
  Command &operator=(Command &&)      = delete;

  virtual ~Command() = default;

  virtual void operator()() = 0;

protected:
  GConf     &gc_ = GConf::get();
  Interface &if_ = Interface::get(); // Could be const since it practically only has const methods

  const bool force_;

  /**
   * @param force
   */
  explicit Command(const bool force = false) : force_(force) {}
};

} // namespace zc
