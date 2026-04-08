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
  virtual int execute() = 0;
};