#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace zc
{

class ShellCommand
{
public:
  ShellCommand() = default;

  void add(const std::string &token);
  [[nodiscard]] std::string string() const;

  int operator()(bool show_output = true) const;

  template<typename T>
  ShellCommand &operator<<(const T &token)
  {
    std::ostringstream ss;
    ss << token;
    add(ss.str());
    return *this;
  }

private:
  std::vector<std::string> tokens_;
};

} // namespace zc
