#include "Interface.h"

#include <cstddef>
#include <iostream>
#include <nlohmann/json.hpp>

#include "../excepts/ZCException.h"
#include "../helpers.h"
#include "excepts/ExitCode.h"
#include "ui/ui_utils.h"

ZC_DEV_CONFIG

namespace zc
{

Interface &Interface::get()
{
  static Interface instance;
  return instance;
}

void Interface::clear() const
{
  if (!quiet_)
    cout << CLEAR_SCREEN << CURSOR_TOP_LEFT << flush;
}

void Interface::flush_screen() const
{
  if (!quiet_)
    cout << flush;
}

void Interface::new_line() const
{
  if (!quiet_)
    cout << endl;
}

void Interface::success(const std::string &message) const
{
  if (!quiet_)
    cout << GREEN "✔ " RESET << message << endl;
}

void Interface::info(const std::string &message) const
{
  if (!quiet_)
    cout << BLUE "➜ " RESET << message << endl;
}

void Interface::print(const std::string &message) const
{
  if (!quiet_)
    cout << message << endl;
}

void Interface::debug(const std::string &message) const
{
  if (!quiet_)
    cout << CYAN "⚙ " RESET << message << endl;
}

void Interface::warning(const std::string &message) const
{
  if (!quiet_)
    cout << YELLOW "! " RESET << message << endl;
}

void Interface::error(const std::string &message) const
{
  if (!quiet_)
    cerr << RED "✗ " RESET << message << endl;
}

bool Interface::ask(const std::string &question, const bool default_ans) const
{
  vector<string> options = { "Yes", "No" };

  size_t selected = radios(question, options, default_ans ? 0 : 1);
  return selected == 0;
}

string Interface::input(const string &question) const
{
  string line;
  cout << BLUE "? " RESET << question << ": " BLUE << flush;

  if (!getline(cin, line))
  {
    cout << RESET;
    if (cin.eof())
      return "";

    cin.clear();
    return "";
  }
  cout << RESET;
  if (line.empty())
  {
    cout << CURSOR_UP(1) << "\r" CLEAR_LINE;
    cout << BLUE "? " RESET << question << ": " << BLUE "none" RESET << endl;
  }
  return line;
}

string Interface::input(const string &question, const string &default_ans) const
{
  string line;
  cout << BLUE "? " RESET << question << " (" << default_ans << "): " BLUE << flush;

  if (!getline(cin, line))
  {
    cout << RESET;
    if (cin.eof())
      return default_ans;
    cin.clear();
    return default_ans;
  }
  cout << RESET;
  if (line.empty())
  {
    cout << CURSOR_UP(1) << "\r" CLEAR_LINE;
    cout << BLUE "? " RESET << question << " (" << default_ans << "): " << BLUE << default_ans << RESET
         << endl;
    return default_ans;
  }
  return line;
}

void Interface::loading_bar(int bar_width, int percent_filled, const std::string &message) const
{
  if (quiet_)
    return;

  string pct_str = std::to_string(percent_filled);
  while (pct_str.length() < 3)
    pct_str = " " + pct_str; // Percentage always takes 3 characters

  const int filled = (percent_filled * bar_width) / 100;
  string    bar    = "";
  for (int i = 0; i < bar_width; i++)
    bar += (i < filled) ? "█" : "░";

  cout << "\r" CLEAR_LINE B_GREEN "◇ " << bar << " " B_GREEN << pct_str << "% " RESET << message << flush;
}

void Interface::clear_loading_bar() const
{
  if (!quiet_)
    cout << "\r" CLEAR_LINE << flush;
}

vector<string> Interface::checkboxes(const string &question, const vector<string> &options) const
{
  size_t       cursor = 0;
  vector<bool> selected(options.size(), false);
  cout << BLUE "? " RESET << question << endl << HIDE_CURSOR;

  set_raw_mode(true);

  while (true)
  {
    for (size_t i = 0; i < options.size(); i++)
    {
      if (i == cursor)
        cout << BLUE "> ";
      else
        cout << WHITE "  ";

      cout << (selected[i] ? "◉ " : "◯ ") << options[i] << RESET << endl;
    }

    const char c = get_char_raw();
    if (c == 3)
    {
      set_raw_mode(false);
      std::cout << SHOW_CURSOR;
      throw ZCException(ZCE_ABORTED, "Interrupted");
    }
    if (c == '\n' || c == '\r')
      break; // exit loop
    if (c == ' ' || c == 'x')
    {
      selected[cursor] = !selected[cursor]; // toggle option
    }
#if defined(_WIN32) || defined(_WIN64)
    else if (c == -32 || c == 0 || c == 224)
    {
      char dir = get_char_raw();
      if (dir == 72 && cursor > 0)
        cursor = (cursor == 0) ? options.size() - 1 : cursor - 1;
      if (dir == 80 && cursor < options.size() - 1)
        cursor = (cursor == options.size() - 1) ? 0 : cursor + 1;
    }
#else
    else if (c == '\033') // Escape sequence
    {
      if (const char bracket = get_char_raw(); bracket == '[')
      {
        const char dir = get_char_raw();
        if (dir == 'A')
          cursor = (cursor == 0) ? options.size() - 1 : cursor - 1;
        if (dir == 'B' && cursor < options.size() - 1)
          cursor = (cursor == options.size() - 1) ? 0 : cursor + 1;
      }
    }
#endif
    cout << CURSOR_UP(options.size()); // Bring the cursor up to erase everything
  }

  set_raw_mode(false);
  cout << SHOW_CURSOR;
  cout << CURSOR_UP(options.size() + 1);
  cout << CLEAR_UNDER_CURSOR;

  vector<string> result;
  for (size_t i = 0; i < options.size(); i++)
    if (selected[i])
      result.push_back(options[i]);

  cout << BLUE << "? " << RESET << question << " " BLUE << join(result, ", ") << RESET << endl;

  return result;
}

size_t Interface::radios(
  const std::string &question, const std::vector<std::string> &options, size_t default_ans
) const
{
  size_t cursor = default_ans;

  cout << BLUE << "? " << RESET << question << endl << HIDE_CURSOR;

  set_raw_mode(true);

  while (true)
  {
    for (size_t i = 0; i < options.size(); i++)
    {
      if (i == cursor)
        cout << BLUE "> ";
      else
        cout << WHITE "  ";

      cout << options[i] << RESET << endl;
    }

    const char c = get_char_raw();

    if (c == 3)
    {
      set_raw_mode(false);
      std::cout << SHOW_CURSOR;
      throw ZCException(ZCE_ABORTED, "Interrupted");
    }
    if (c == '\n' || c == '\r')
      break;
#if defined(_WIN32) || defined(_WIN64)
    else if (c == -32 || c == 0 || c == 224)
    {
      char dir = get_char_raw();
      if (dir == 72 && cursor > 0)
        cursor = (cursor == 0) ? options.size() - 1 : cursor - 1;
      if (dir == 80 && cursor < options.size() - 1)
        cursor = (cursor == options.size() - 1) ? 0 : cursor + 1;
    }
#else
    if (c == '\033')
    {
      if (const char bracket = get_char_raw(); bracket == '[')
      {
        const char dir = get_char_raw();
        if (dir == 'A' && cursor > 0)
          cursor = (cursor == 0) ? options.size() - 1 : cursor - 1;
        if (dir == 'B' && cursor < options.size() - 1)
          cursor = (cursor == options.size() - 1) ? 0 : cursor + 1;
      }
    }
#endif
    cout << CURSOR_UP(options.size());
  }

  set_raw_mode(false);
  cout << SHOW_CURSOR;

  cout << CURSOR_UP(options.size() + 1);
  cout << CLEAR_UNDER_CURSOR;
  cout << BLUE << "? " << RESET << question << " " BLUE << options[cursor] << RESET << endl;

  return cursor;
}

} // namespace zc
