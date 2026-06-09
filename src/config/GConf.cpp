/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include "GConf.h"

/**
 * GConf implementation
 *
 * ZC Global Configuration
 */

/**
 * @return GConf
 */
GConf &GConf::get()
{
  static GConf instance;
  return instance;
}

/**
 * @return void
 */
void GConf::login()
{
  return;
}

/**
 * @return void
 */
void GConf::logout()
{
  return;
}

/**
 * @return void
 */
void GConf::load()
{
  return;
}

/**
 * @return void
 */
void GConf::write()
{
  return;
}

GConf::GConf()
{
}
