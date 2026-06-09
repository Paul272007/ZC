/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#ifndef _INTERFACE_H
#define _INTERFACE_H

#include <string>

class Interface
{
public:
  /**
   * @param bool quiet
   */
  static Interface &get(bool quiet = false);

  void clear();

  void success();

  void info();

  void debug();

  void warning();

  void error();

  /**
   * @param question
   */
  bool ask(const std::string &question);

  /**
   * @param question
   * @param default
   */
  std::string input(const std::string &question, const std::string &default_ans);

private:
  const bool quiet_;

  /**
   * @param quiet
   */
  Interface(bool quiet);
};

#endif //_INTERFACE_H
