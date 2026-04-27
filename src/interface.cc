#include <iostream>
#include <string>

#include "interface.hh"

using namespace std;

void success(const string &msg)
{
  cout << "[" GREEN "SUCCESS" COLOR_RESET "] " << msg << endl;
}

void debug(const string &msg)
{
  cout << "[" CYAN "DEBUG" COLOR_RESET "]   " << msg << endl;
}

void warning(const string &msg)
{
  cout << "[" YELLOW "WARNING" COLOR_RESET "] " << msg << endl;
}

void info(const string &msg)
{
  cout << "[" BLUE "INFO" COLOR_RESET "]    " << msg << endl;
}

void error(const string &msg)
{
  cerr << "[" RED "ERROR" COLOR_RESET "]   " << msg << endl;
}

void clear_screen()
{
  // \033[2J = clear screen
  // \033[1;1H = set cursor at the top left corner
  std::cout << "\033[2J\033[1;1H" << std::flush;
}

bool ask(const string &question)
{
  string line;
  cout << question << endl << "[Y/n] ";

  while (getline(cin, line))
  {
    // 1. If the line is empty, we consider it as 'yes'
    if (line.empty())
      return true;

    // 2. Otherwise, we check the first character
    char input = toupper(line[0]);

    if (input == 'Y')
      return true;
    if (input == 'N')
      return false;

    cout << "Error: unexpected token" << endl << "[Y/n] ";
  }

  return true; // Security if the input stream is closed
}

string input(const std::string &question)
{
  string line;
  cout << question << "\n> " << flush;

  if (!getline(cin, line))
  {
    if (cin.eof())
      return "";

    cin.clear();
    return "";
  }

  return line;
}

string input(const std::string &question, const std::string &default_answer)
{
  string line;
  cout << question << "\n(" << default_answer << ")> " << flush;

  if (!getline(cin, line))
  {
    if (cin.eof())
      return default_answer;

    cin.clear();
    return default_answer;
  }

  if (line.empty())
    return default_answer;
  return line;
}

void fl()
{
  std::cout << std::flush;
}

void nl()
{
  std::cout << '\n';
}
