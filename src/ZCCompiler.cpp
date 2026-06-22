#include "ZCCompiler.h"

#include <sstream>

#include "helpers.h"

ZC_DEV_CONFIG

ZCCompiler::ZCCompiler(
  const std::string &compiler, const std::vector<std::string> &flags,
  const std::vector<std::string> &sources
)
  : compiler_(compiler), flags_(flags), sources_(sources)
{
}

void ZCCompiler::operator()()
{
  stringstream command;

  command << compiler_ << " ";
  for (const auto &src : sources_)
    command << src << " ";
  for (const auto &flag : flags_)
    command << flag << " ";

  zc::exec_command(command.str());
}
