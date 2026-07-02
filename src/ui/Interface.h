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
  Interface(Interface &&)                 = delete;
  Interface &operator=(Interface &&)      = delete;
  Interface(const Interface &)            = delete;
  Interface &operator=(const Interface &) = delete;
  ~Interface()                            = default;

  void clear() const;
  void new_line() const;
  void flush_screen() const;

  void info(const std::string &message) const;
  void print(const std::string &message) const;
  void debug(const std::string &message) const;
  void error(const std::string &message) const;
  void success(const std::string &message) const;
  void warning(const std::string &message) const;

  /**
   * @param question
   * @param default_ans
   */
  [[nodiscard]] bool ask(const std::string &question, bool default_ans = true) const;

  [[nodiscard]] std::string input(const std::string &question) const;
  [[nodiscard]] std::string input(const std::string &question, const std::string &default_ans) const;
  [[nodiscard]] std::vector<std::string>
  input_list(const std::string &question, const std::vector<std::string> &default_ans) const;

  void loading_bar(int bar_width, int percent_filled, const std::string &message) const;

  void clear_loading_bar() const;

  void checkboxes(
    const std::string &question, const std::vector<std::string> &options, std::vector<bool> &selected
  ) const;

  [[nodiscard]] size_t radios(
    const std::string &question, const std::vector<std::string> &options, size_t default_ans = 0
  ) const;

  [[nodiscard]] bool is_quiet() const { return quiet_; }

  void set_quiet(const bool quiet) { quiet_ = quiet; }

private:
  bool quiet_ = false;

  Interface() = default;
};

} // namespace zc
