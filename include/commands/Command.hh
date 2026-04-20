#pragma once

#include <string>

#include <interface.hh>

class Command
{
public:
  /**
   * @brief Create default destructor
   */
  virtual ~Command() = default;

  /**
   * @brief Execute the Command
   *
   * Get overwritten in every child class
   *
   * @return Exit code of the command
   */
  virtual int operator()() = 0;

protected:
  Command(const bool force, const bool quiet) : force_(force), quiet_(quiet)
  {
  }

  void log_info(const std::string &message) const
  {
    if (!quiet_)
      info(message);
  }

  void log_success(const std::string &message) const
  {
    if (!quiet_)
      info(message);
  }

  void log_debug(const std::string &message) const
  {
    if (!quiet_)
      debug(message);
  }

  void log_warning(const std::string &message) const
  {
    if (!quiet_)
      debug(message);
  }

  const bool force_;
  const bool quiet_;
};
