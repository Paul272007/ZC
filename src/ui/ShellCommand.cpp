#include "ShellCommand.h"

#include <string>

#include "helpers.h"
#include "ui/Interface.h"
#include "ui/ui_utils.h"

ZC_DEV_CONFIG

namespace zc
{

int ShellCommand::exec(const std::vector<std::string> &tokens, output style)
{
  return ShellCommand{ tokens }(style);
}

ShellCommand::ShellCommand(const std::vector<std::string> &tokens)
{
  for (const auto &t : tokens)
    add(t);
}

void ShellCommand::add(const std::string &token)
{
  tokens_.push_back(esc(token));
}

int ShellCommand::operator()(output style) const
{
  std::string cmd = string() + get_suffix(style);
  ui().debug("Execute command: " + cmd);
  return system(cmd.c_str());
}

string ShellCommand::string() const
{
  return join(tokens_, " ");
}

FILE *ShellCommand::pipe(output style) const
{
  std::string cmd = string() + get_suffix(style);
  ui().debug("Open pipe: " + cmd);
  FILE *pipe = popen(cmd.c_str(), "r");
  if (pipe == nullptr)
    throw ZCException(ZCE_INTERNAL_ERROR, "Failed to execute command '" + cmd + "'");
  return pipe;
}

} // namespace zc
