/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include "Interface.h"
#include "../helpers.h"

ZC_DEV_CONFIG

/**
 * Interface implementation
 */

/**
 * @param bool quiet
 * @return Interface
 */
Interface &Interface::get(bool quiet)
{
  static Interface instance(quiet);
  return instance;
}

/**
 * @return void
 */
void Interface::clear()
{
  return;
}

/**
 * @return void
 */
void Interface::success()
{
  return;
}

/**
 * @return void
 */
void Interface::info()
{
  return;
}

/**
 * @return void
 */
void Interface::debug()
{
  return;
}

/**
 * @return void
 */
void Interface::warning()
{
  return;
}

/**
 * @return void
 */
void Interface::error()
{
  return;
}

/**
 * @param question
 * @return bool
 */
bool Interface::ask(const std::string &question)
{
  return false;
}

/**
 * @param question
 * @param default
 * @return string
 */
string Interface::input(const string &question, const string &default_ans)
{
  return "";
}

/**
 * @param quiet
 */
Interface::Interface(bool quiet) : quiet_(quiet)
{
}
