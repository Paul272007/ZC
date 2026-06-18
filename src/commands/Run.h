/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <string>
#include <vector>

#include "../CompileMode.h"
#include "Command.h"

namespace zc
{

class Run : public Command
{
public:
  Run(bool force);

  int operator()() override;

private:
  CompileMode mode_;

  bool has_cpp();

  void build_command();

  std::vector<std::string> get_dependencies();
};

} // namespace zc
