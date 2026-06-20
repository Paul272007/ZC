/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#pragma once

#include <filesystem>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace zc
{

class Interface
{
public:
  static Interface &get();
  Interface(const Interface &) = delete;
  void operator=(const Interface &) = delete;

  void set_quiet(bool quiet)
  {
    quiet_ = quiet;
  }

  void clear() const;

  void flush() const;

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

  void write_json(const nlohmann::json &json, const std::filesystem::path &file_path) const;

  nlohmann::json read_json(const std::filesystem::path &file_path) const;

  bool is_quiet() const
  {
    return quiet_;
  }

private:
  bool quiet_ = false;

  Interface() = default;
};

} // namespace zc
