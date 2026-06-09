/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */


#include "Run.h"

/**
 * Run implementation
 */


void Run::Run() {

}

/**
 * @return int
 */
int Run::operator()() {
  return 0;
}

/**
 * @return bool
 */
bool Run::has_cpp() {
  return false;
}

/**
 * @return void
 */
void Run::build_command() {
  return;
}

/**
 * @return vector<string>
 */
vector<string> Run::get_dependencies() {
  return null;
}