#pragma once

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#ifndef _WIN32
  #include <sys/wait.h>
#endif

#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"

namespace zc
{

enum class output : std::uint8_t
{
  show_all,
  hide_out,
  hide_err,
  err_to_out,
  out_to_err,
  hide_all,
};

class ShellCommand
{
public:
  static int exec(const std::vector<std::string> &tokens, output style = output::show_all);

  ShellCommand() = default;
  ShellCommand(const std::vector<std::string> &tokens);

  void add(const std::string &token);

  [[nodiscard]] std::string string() const;
  [[nodiscard]] FILE *pipe(output style = output::show_all) const;

  int output_actions(
    size_t buffer_size, auto callback, output style = output::show_all,
    const std::string &error_msg = "Command failed"
  ) const
  {
    FILE             *cmd_pipe = pipe(style);
    std::vector<char> buffer(buffer_size);

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), cmd_pipe) != nullptr)
    {
      std::string line{ buffer.data() };
      callback(line);
    }

    const int result    = pclose(cmd_pipe);
    int       exit_code = result;
#ifndef _WIN32
    if (WIFEXITED(result))
      exit_code = WEXITSTATUS(result);
#endif

    if (exit_code != 0)
      throw ZCException(ZCE_INTERNAL_ERROR, error_msg + " (exit code " + std::to_string(exit_code) + ")");

    return exit_code;
  }

  int operator()(output style = output::show_all) const;

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
