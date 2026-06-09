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
  Interface if_;

  /**
   * @param force
   */
  Command(bool force = false);

private:
  GConf gconf_;
};

#endif //_COMMAND_H
