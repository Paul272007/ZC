/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _RUN_H
#define _RUN_H

#include <string>
#include <vector>

#include "../CompileMode.h"
#include "Command.h"

class Run : public Command
{
public:
  Run();

  int operator()() override;

private:
  CompileMode mode_;

  bool has_cpp();

  void build_command();

  std::vector<std::string> get_dependencies();
};

#endif //_RUN_H
