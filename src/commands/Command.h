/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include "../config/GConf.h"
#include "../ui/Interface.h"

namespace zc
{

class Command
{
public:
  virtual ~Command() = default;

  virtual void operator()() = 0;

protected:
  const bool force_;
  const Interface &if_ = Interface::get();
  const GConf &gconf_ = GConf::get();

  /**
   * @param force
   */
  explicit Command(const bool force = false) : force_(force)
  {
  }
};

} // namespace zc
