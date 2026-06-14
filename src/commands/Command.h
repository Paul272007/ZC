/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _COMMAND_H
#define _COMMAND_H

#include "../config/GConf.h"
#include "../ui/Interface.h"

class Command
{
public:
  virtual ~Command() = 0;

  virtual int operator()() = 0;

protected:
  const bool force_ = false;
  const Interface &if_ = Interface::get();
  const GConf &gconf_ = GConf::get();

  /**
   * @param force
   */
  explicit Command(const bool force = false) : force_(force)
  {
  }
};

#endif //_COMMAND_H
