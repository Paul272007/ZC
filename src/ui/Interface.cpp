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

ZC_DEV_CONFIG

/**
 * Interface implementation
 */

Interface &Interface::get(const bool quiet)
{
  static Interface instance(quiet);
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

/**
 * @return void
 */
void Interface::success(const std::string &message) const
{
  if (!quiet_)
    cout << "[" GREEN "SUCCESS" RESET "] " << message << endl;
}

/**
 * @return void
 */
void Interface::info(const std::string &message) const
{
  if (!quiet_)
    cout << "[" BLUE "INFO" RESET "]    " << message << endl;
}

/**
 * @return void
 */
void Interface::debug(const std::string &message) const
{
  if (!quiet_)
    cout << "[" CYAN "DEBUG" RESET "]   " << message << endl;
}

/**
 * @return void
 */
void Interface::warning(const std::string &message) const
{
  if (!quiet_)
    cout << "[" YELLOW "WARNING" RESET "] " << message << endl;
}

/**
 * @return void
 */
void Interface::error(const std::string &message) const
{
  if (!quiet_)
    cerr << "[" RED "ERROR" RESET "]   " << message << endl;
}

bool Interface::ask(const std::string &question, const bool default_ans) const
{
  string line;
  cout << question << endl << (default_ans ? "[Y/n] " : "[y/N] ");

  while (getline(cin, line))
  {
    // 1. If the line is empty, we consider it as 'yes'
    if (line.empty())
      return true;

    // 2. Otherwise, we check the first character
    const char input = toupper(line[0]);

    if (input == 'Y')
      return true;
    if (input == 'N')
      return false;

    cout << "Error: unexpected token" << endl << "[Y/n] ";
  }

  return true; // Security if the input stream is closed
}

string Interface::input(const string &question, const string &default_ans) const
{
  string line;
  cout << question << "\n(" << default_ans << ")> " << std::flush;

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

/**
 * @param quiet
 */
Interface::Interface(const bool quiet) : quiet_(quiet)
{
}
