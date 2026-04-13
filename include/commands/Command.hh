#pragma once

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

  const bool force_;
  const bool quiet_;
};
