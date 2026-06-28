#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace zc
{

class Interface
{
public:
  static Interface &get();
  Interface(const Interface &)      = delete;
  void operator=(const Interface &) = delete;

  void clear() const;

  void flush_screen() const;

  void new_line() const;

  void success(const std::string &message) const;

  void info(const std::string &message) const;

  void print(const std::string &message) const;

  void debug(const std::string &message) const;

  void warning(const std::string &message) const;

  void error(const std::string &message) const;

  /**
   * @param question
   * @param default_ans
   */
  bool ask(const std::string &question, bool default_ans = true) const;

  std::string input(const std::string &question) const;

  /**
   * @param question The question to be answered
   * @param default_ans The default answer if no one was given
   */
  std::string input(const std::string &question, const std::string &default_ans) const;

  void loading_bar(int bar_width, int percent_filled, const std::string &message) const;

  void clear_loading_bar() const;

  std::vector<std::string>
  checkboxes(const std::string &question, const std::vector<std::string> &options) const;

  size_t radios(
    const std::string &question, const std::vector<std::string> &options, size_t default_ans = 0
  ) const;

  bool is_quiet() const { return quiet_; }

  void set_quiet(const bool quiet) { quiet_ = quiet; }

private:
  bool quiet_ = false;

  Interface() = default;
};

} // namespace zc
