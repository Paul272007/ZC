/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include "Command.h"

/**
 * Command implementation
 */

Command::~Command()
{
}

/**
 * @return int
 */
int Command::operator()()
{
  return 0;
}

/**
 * @param force
 */
Command::Command(bool force)
{
}
