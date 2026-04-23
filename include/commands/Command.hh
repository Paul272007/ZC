#pragma once

#include <functional>
#include <string>

#include "interface.hh"
#include "objects/Controllers/Controller.hh"

class Command
{
public:
  virtual ~Command() = default;

  virtual int operator()() = 0;

protected:
  Command(const bool force, const bool quiet) : force_(force), quiet_(quiet)
  {
  }

  const bool force_;
  const bool quiet_;

  std::function<void(LogLevel, const std::string &)> logger_ = [this](LogLevel level, const std::string &msg)
  {
    if (this->quiet_)
      return;

    switch (level)
    {
    case LogLevel::INFO:
      info(msg);
      break;
    case LogLevel::SUCCESS:
      success(msg);
      break;
    case LogLevel::WARNING:
      warning(msg);
      break;
    case LogLevel::DEBUG:
      debug(msg);
      break;
    case LogLevel::ERROR:
      error(msg);
      break;
    }
  };
};
