#include "framework/util/string.hpp"

#include <sstream>

namespace zc
{

std::string quote(const std::string &str)
{
  return "\"" + str + "\"";
}

std::string join(const std::vector<std::string> &v, const std::string &separator)
{
  std::stringstream s;
  for (size_t i = 0; i < v.size(); i++) {
    if (i != 0)
      s << separator;
    s << v.at(i);
  }
  return s.str();
}

std::string upper(const std::string &text)
{
  std::string output;
  output.reserve(text.size());
  for (const auto &c : text)
    output += static_cast<char>(toupper(static_cast<unsigned char>(c)));
  return output;
}

std::string lower(const std::string &text)
{
  std::string output;
  output.reserve(text.size());
  for (const auto &c : text)
    output += static_cast<char>(tolower(static_cast<unsigned char>(c)));
  return output;
}

std::string esc(const std::string &arg)
{
  std::string escaped;
  for (const char c : arg)
    if (c == '\'')
      escaped += "'\\''";
    else
      escaped += c;
  return "'" + escaped + "'";
}

std::vector<std::string> split(const std::string &str, char delimiter)
{
  std::vector<std::string> tokens;

  size_t start = 0;
  size_t end   = str.find(delimiter);

  while (end != std::string::npos) {
    if (end != start)
      tokens.push_back(str.substr(start, end - start));
    start = end + 1;
    end   = str.find(delimiter, start);
  }

  if (start < str.length())
    tokens.push_back(str.substr(start));

  return tokens;
}

} // namespace zc
