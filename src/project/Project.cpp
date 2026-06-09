/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include "Project.h"
#include "../helpers.h"

ZC_DEV_CONFIG

/**
 * Project implementation
 */

Project::Project() : reg_(Registry::get())
{
}

Project::~Project()
{
}

/**
 * @param release
 * @return void
 */
void Project::build(bool release)
{
  return;
}

/**
 * @return void
 */
void Project::clean()
{
  return;
}

/**
 * @return void
 */
void Project::publish()
{
  return;
}

/**
 * @param target
 * @return bool
 */
bool Project::add_dependency(const string &target)
{
  return false;
}

/**
 * @param target
 * @return bool
 */
bool Project::remove_dependency(const string &target)
{
  return false;
}

/**
 * @return void
 */
void Project::generate_Makefile()
{
  return;
}

/**
 * @return void
 */
void Project::generate_compile_commands()
{
  return;
}
