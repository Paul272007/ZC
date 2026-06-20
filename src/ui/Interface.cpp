/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "../excepts/ZCException.h"
#include "../helpers.h"
#include "Interface.h"
#include "ui/ui_utils.h"

ZC_DEV_CONFIG

namespace zc
{

/**
 * Interface implementation
 */

Interface &Interface::get()
{
  static Interface instance;
  return instance;
}

void Interface::clear() const
{
  // \033[2J = clear screen
  // \033[1;1H = set cursor in the top left corner
  if (!quiet_)
    std::cout << "\033[2J\033[1;1H" << std::flush;
}

void Interface::flush() const
{
  if (!quiet_)
    cout << std::flush;
}

void Interface::new_line() const
{
  if (!quiet_)
    cout << std::endl;
}

void Interface::success(const std::string &message) const
{
  if (!quiet_)
    cout << "[" GREEN "SUCCESS" RESET "] " << message << endl;
}

void Interface::info(const std::string &message) const
{
  if (!quiet_)
    cout << "[" BLUE "INFO" RESET "]    " << message << endl;
}

void Interface::print(const std::string &message) const
{
  if (!quiet_)
    cout << message << endl;
}

void Interface::debug(const std::string &message) const
{
  if (!quiet_)
    cout << "[" CYAN "DEBUG" RESET "]   " << message << endl;
}

void Interface::warning(const std::string &message) const
{
  if (!quiet_)
    cout << "[" YELLOW "WARNING" RESET "] " << message << endl;
}

void Interface::error(const std::string &message) const
{
  if (!quiet_)
    cerr << "[" RED "ERROR" RESET "]   " << message << endl;
}

bool Interface::ask(const std::string &question, const bool default_ans) const
{
  string line;
  cout << BLUE "? " RESET << question << " [" << (default_ans ? "Y/n" : "y/N") << "] " << std::flush;

  while (getline(cin, line))
  {
    if (line.empty())
      return default_ans;

    const char input = toupper(line[0]);
    if (input == 'Y')
      return true;
    if (input == 'N')
      return false;

    cout << "Error: unexpected token" << endl << "[" << (default_ans ? "Y/n" : "y/N") << "] " << std::flush;
  }
  return default_ans; // Security if the input stream is closed
}

string Interface::input(const string &question) const
{
  string line;
  cout << BLUE "? " RESET << question << " : " << std::flush;

  if (!getline(cin, line))
  {
    if (cin.eof())
      return "";

    cin.clear();
    return "";
  }

  return line;
}

string Interface::input(const string &question, const string &default_ans) const
{
  string line;
  cout << BLUE "? " RESET << question << " (" << default_ans << "): " << std::flush;

  if (!getline(cin, line))
  {
    if (cin.eof())
      return default_ans;

    cin.clear();
    return default_ans;
  }

  if (line.empty())
    return default_ans;
  return line;
}

void Interface::loading_bar(int bar_width, int percent_filled, const std::string &message) const
{
  if (quiet_)
    return;

  // Percentage always takes 3 characters
  string pct_str = std::to_string(percent_filled);
  while (pct_str.length() < 3) pct_str = " " + pct_str;

  // Loading bar
  int filled = (percent_filled * bar_width) / 100;
  string bar = "";
  for (int i = 0; i < bar_width; i++) bar += (i < filled) ? "█" : "░";

  cout << "\r" CLEAR_LINE << "[" B_GREEN << bar << RESET "] " B_GREEN << pct_str << "% " RESET << message
       << std::flush;
}

void Interface::clear_loading_bar() const
{
  if (!quiet_)
    std::cout << "\r" CLEAR_LINE << std::flush;
}

nlohmann::json Interface::read_json(const std::filesystem::path &file_path) const
{
  nlohmann::json parsed_json;

  if (!std::filesystem::exists(file_path))
    throw ZCException(ZCE_NOT_FOUND, "The JSON file was not found: " + file_path.string());

  std::ifstream input(file_path);
  if (!input.is_open())
    throw ZCException(ZCE_READING_ERROR, "The JSON file couldn't be read: " + file_path.string());

  try
  {
    input >> parsed_json;
  }
  catch (const nlohmann::json::parse_error &e)
  {
    throw ZCException(
        ZCE_PARSING_ERROR, "The JSON file couldn't be parsed: " + file_path.string() + ": " + e.what()
    );
  }

  return parsed_json;
}

void Interface::write_json(const nlohmann::json &json, const std::filesystem::path &file_path) const
{
  ofstream output(file_path);
  if (!output.is_open())
    throw ZCException(ZCE_WRITING_ERROR, "The JSON file couldn't be written: " + file_path.string());

  output << json.dump(2);
  output.close();
}

} // namespace zc
