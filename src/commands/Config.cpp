#include "Config.h"

#include "commands/Command.h"
#include "helpers.h"

ZC_DEV_CONFIG

namespace zc
{

Config::Config(bool force, std::string key, std::string value)
  : Command(force), key_(std::move(key)), value_(std::move(value))
{
}

void Config::operator()()
{
  if (key_ == "edit")
    gc_.edit_config(force_);
  else if (key_ == "default")
    gc_.default_config(force_);
  else
    gc_.set(key_, value_);
}

} // namespace zc
