#include "ShellCommand.h"

#include <string>

#include "helpers.h"
#include "ui/Interface.h"

ZC_DEV_CONFIG

namespace zc
{

void ShellCommand::add(const std::string &token)
{
  tokens_.push_back(esc(token));
}

int ShellCommand::operator()(bool show_output) const
{
  std::string cmd = string() + (show_output ? "" : HIDE_OUTPUT);
  ui().debug(cmd);
  return system(cmd.c_str());
}

string ShellCommand::string() const
{
  return join(tokens_, " ");
}

} // namespace zc
