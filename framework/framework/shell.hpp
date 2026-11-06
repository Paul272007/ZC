#pragma once

#include <cstdio>
#include <cstdlib>
#include <format>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

#ifndef ZC_WINDOWS
  #include <sys/wait.h>
#endif

#include "framework/Exception.hpp"
#include "util/concepts.hpp"
#include "util/string.hpp"
#include "util/types.hpp"

namespace zc::sh
{

#ifdef ZC_WINDOWS
inline constexpr std::string_view hide_all = " > NUL 2>&1";
// TODO: add other variables for windows
#else
inline constexpr std::string_view hide_all         = ">/dev/null 2>&1";
inline constexpr std::string_view hide_stdout      = "1>/dev/null";
inline constexpr std::string_view hide_stderr      = "2>/dev/null";
inline constexpr std::string_view stderr_to_stdout = "2>&1";
inline constexpr std::string_view stdout_to_stderr = "1>&2";
#endif

enum class Output : u8 {
  ShowAll,
  HideAll,
  HideStdout,
  HideStderr,
  StderrToStdout,
  StdoutToStderr,
};

[[nodiscard]] constexpr std::string_view suffix(const Output output)
{
  switch (output) {
  case Output::HideAll:
    return hide_all;
  case Output::HideStderr:
    return hide_stderr;
  case Output::HideStdout:
    return hide_stdout;
  case Output::StderrToStdout:
    return stderr_to_stdout;
  case Output::StdoutToStderr:
    return stdout_to_stderr;
  case Output::ShowAll:
  default:
    return "";
  }
}

class Cmd
{
  const Output             output_;
  std::vector<std::string> tokens_;

public:
  template<Formattable T>
  Cmd(const std::initializer_list<T> &tokens, const Output output) : output_(output)
  {
    for (const auto &token : tokens)
      *this << token;
  }

  template<Formattable T>
  Cmd &operator<<(T val)
  {
    tokens_.push_back(std::format("{}", val));
    return *this;
  }

  [[nodiscard]] std::string string() const { return join(tokens_, " ") + " " + suffix(output_); }

  int operator()() const
  {
    int exit_code = std::system(string().c_str());
#ifndef ZC_WINDOWS
    if (WIFEXITED(exit_code))
      return WEXITSTATUS(exit_code);

    if (WIFSIGNALED(exit_code))
      return 128 + WTERMSIG(exit_code);
#endif
    return exit_code;
  }

  [[nodiscard]] std::FILE *pipe() const
  {
    const std::string cmd = string();

    FILE *pipe = popen(cmd.c_str(), "r");
    if (!pipe)
      throw Exception(OSError::PipeError, "Failed to open pipe for command: ", cmd);
    return pipe;
  }

  [[nodiscard]] int forEachLine(const u64 buffer_size, auto callback)
  {
    FILE             *cmd_pipe = pipe();
    std::vector<char> buffer(buffer_size);

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), cmd_pipe)) {
      std::string line{ buffer.data() };
      callback(line);
    }
    int exit_code = pclose(cmd_pipe);
#ifndef ZC_WINDOWS
    if (WIFEXITED(exit_code))
      return WEXITSTATUS(exit_code);

    if (WIFSIGNALED(exit_code))
      return 128 + WTERMSIG(exit_code);
#endif
    return exit_code;
  }
};

template<Formattable T>
int exec(const std::initializer_list<T> &tokens, const Output output = Output::ShowAll)
{
  return Cmd(tokens, output)();
}

template<Formattable T>
std::FILE *pipe(const std::initializer_list<T> &tokens, const Output output = Output::ShowAll)
{
  return Cmd(tokens, output).pipe();
}

template<Formattable T>
int forEachLine(
  const std::initializer_list<T> &tokens, const u64 buffer_size, auto callback,
  const Output output = Output::ShowAll
)
{
  return Cmd(tokens, output).forEachLine(buffer_size, callback);
}

} // namespace zc::sh
