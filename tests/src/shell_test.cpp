#include "framework/shell.hpp"

#include <gtest/gtest.h>

namespace zc::tests
{

TEST(ShellTest, Suffix)
{
  EXPECT_EQ(sh::suffix(sh::Output::ShowAll), "");
#ifndef ZC_WINDOWS
  EXPECT_EQ(sh::suffix(sh::Output::HideAll), ">/dev/null 2>&1");
  EXPECT_EQ(sh::suffix(sh::Output::HideStdout), "1>/dev/null");
  EXPECT_EQ(sh::suffix(sh::Output::HideStderr), "2>/dev/null");
  EXPECT_EQ(sh::suffix(sh::Output::StderrToStdout), "2>&1");
  EXPECT_EQ(sh::suffix(sh::Output::StdoutToStderr), "1>&2");
#endif
}

TEST(ShellTest, CmdBuilding)
{
  sh::Cmd cmd({ "git", "status" }, sh::Output::HideAll);
  cmd << "-s";
#ifndef ZC_WINDOWS
  EXPECT_EQ(cmd.string(), "git status -s >/dev/null 2>&1");
#endif
}

TEST(ShellTest, Exec)
{
  int status = sh::exec({ "echo", "zc_shell_test" }, sh::Output::HideAll);
  EXPECT_EQ(status, 0);
}

TEST(ShellTest, ForEachLine)
{
  std::vector<std::string> lines;
  int                      code = sh::forEachLine(
    { "printf", "'line1\\nline2\\n'" }, 128, [&](const std::string &line) { lines.push_back(line); },
    sh::Output::ShowAll
  );

  EXPECT_EQ(code, 0);
  ASSERT_EQ(lines.size(), 2);
  EXPECT_EQ(lines[0], "line1\n");
  EXPECT_EQ(lines[1], "line2\n");
}

} // namespace zc::tests
