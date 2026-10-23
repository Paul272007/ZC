#include <gtest/gtest.h>

#include "framework/cli/AppBuilder.hpp"
#include "framework/cli/ArgStream.hpp"
#include "framework/cli/CommandBuilder.hpp"
#include "framework/cli/FixedString.hpp"
#include "framework/cli/Option.hpp"
#include "framework/cli/Positional.hpp"

namespace zc::tests
{

TEST(CliTest, FixedString)
{
  constexpr cli::FixedString str1 = "hello";
  constexpr cli::FixedString str2 = "hello";
  constexpr cli::FixedString str3 = "world";

  EXPECT_EQ(str1.sv(), "hello");
  EXPECT_TRUE(str1 == str2);
  EXPECT_FALSE(str1 == str3);

  constexpr cli::FixedString longer = "hello world";
  EXPECT_FALSE(str1 == longer);
}

TEST(CliTest, ArgStream)
{
  const char *argv[] = { "prog", "--flag", "val", "pos" };
  int         argc   = 4;

  cli::ArgStream stream{ .argc = argc, .argv = argv };
  EXPECT_TRUE(stream.hasNext());
  EXPECT_EQ(stream.current(), "--flag");
  EXPECT_EQ(stream.peek(), "val");

  stream.advance();
  EXPECT_TRUE(stream.hasNext());
  EXPECT_EQ(stream.current(), "val");
  EXPECT_EQ(stream.peek(), "pos");

  stream.advance(2);
  EXPECT_FALSE(stream.hasNext());
  EXPECT_EQ(stream.peek(), "");
}

TEST(CliTest, OptionAndPositionalStructures)
{
  int  val = 0;
  auto o1  = cli::opt<int, "jobs", 'j'>(val, "Number of jobs");
  EXPECT_EQ(o1.longName(), "jobs");
  EXPECT_EQ(o1.shortName(), 'j');
  EXPECT_TRUE(o1.hasLongName());
  EXPECT_TRUE(o1.hasShortName());
  EXPECT_FALSE(o1.isRequired());
  EXPECT_FALSE(o1.hasImplicitValue());

  auto o2 = cli::opt_with_default<int, "threads", 't'>(val, 8, "Thread count");
  EXPECT_TRUE(o2.hasImplicitValue());
  EXPECT_EQ(o2.implicitValue(), 8);

  auto o3 = cli::required_opt<int, "count">(val, "Count");
  EXPECT_TRUE(o3.isRequired());
  EXPECT_FALSE(o3.hasShortName());

  std::string file;
  auto        p1 = cli::pos<std::string, "file">(file, "Input file");
  EXPECT_EQ(p1.name(), "file");
  EXPECT_FALSE(p1.isRequired());

  auto p2 = cli::required_pos<std::string, "dest">(file, "Destination");
  EXPECT_EQ(p2.name(), "dest");
  EXPECT_TRUE(p2.isRequired());
}

TEST(CliTest, DetailParseValue)
{
  int int_val = 0;
  EXPECT_TRUE(cli::detail::parse_value("123", int_val));
  EXPECT_EQ(int_val, 123);
  EXPECT_FALSE(cli::detail::parse_value("abc", int_val));

  double double_val = 0.0;
  EXPECT_TRUE(cli::detail::parse_value("3.14", double_val));
  EXPECT_NEAR(double_val, 3.14, 0.001);

  bool bool_val = false;
  EXPECT_TRUE(cli::detail::parse_value("true", bool_val));
  EXPECT_TRUE(bool_val);
  EXPECT_TRUE(cli::detail::parse_value("0", bool_val));
  EXPECT_FALSE(bool_val);

  std::string_view sv_val;
  EXPECT_TRUE(cli::detail::parse_value("hello", sv_val));
  EXPECT_EQ(sv_val, "hello");
}

TEST(CliTest, CombinedFlags)
{
  bool a = false;
  bool b = false;
  bool c = false;

  auto opt_a = cli::opt<bool, "all", 'a'>(a);
  auto opt_b = cli::opt<bool, "backup", 'b'>(b);
  auto opt_c = cli::opt<bool, "compress", 'c'>(c);

  auto opts  = std::make_tuple(opt_a, opt_b);
  auto globs = std::make_tuple(opt_c);

  EXPECT_TRUE(cli::detail::try_combined_flags(opts, globs, "-abc"));
  EXPECT_TRUE(a);
  EXPECT_TRUE(b);
  EXPECT_TRUE(c);

  EXPECT_FALSE(cli::detail::try_combined_flags(opts, globs, "-abx"));
}

TEST(CliTest, CommandParsingFlagsAndPositionals)
{
  bool             verbose = false;
  int              jobs    = 1;
  std::string_view input;
  bool             action_executed = false;

  auto my_cmd = cli::cmd<"build", "Build project">()
                  .flag<"verbose", 'v'>(verbose, "Verbose output")
                  .flag<"jobs", 'j'>(jobs, "Concurrent jobs")
                  .positional<"input">(input, "Input source")
                  .setAction([&] { action_executed = true; });

  const char    *argv[] = { "build", "-v", "--jobs=4", "main.cpp" };
  cli::ArgStream stream{ .argc = 4, .argv = argv };
  std::tuple<>   empty;

  my_cmd.parseStream(stream, empty);

  EXPECT_TRUE(verbose);
  EXPECT_EQ(jobs, 4);
  EXPECT_EQ(input, "main.cpp");
  EXPECT_TRUE(action_executed);
}

TEST(CliTest, SubcommandDispatch)
{
  bool             sub_executed = false;
  std::string_view remote_name;

  auto push_cmd = cli::cmd<"push", "Push changes">().positional<"remote">(remote_name).setAction([&] {
    sub_executed = true;
  });

  auto root_cmd = cli::cmd<"git", "Version control">().subcommand(push_cmd).requiresSubcommand();

  const char    *argv[] = { "git", "push", "origin" };
  cli::ArgStream stream{ .argc = 3, .argv = argv };
  std::tuple<>   empty;

  root_cmd.parseStream(stream, empty);

  EXPECT_TRUE(sub_executed);
  EXPECT_EQ(remote_name, "origin");
}

TEST(CliTest, MissingRequiredFlagThrows)
{
  int  required_val = 0;
  auto my_cmd       = cli::cmd<"test", "Test command">().requiredFlag<"mandatory">(required_val);

  const char    *argv[] = { "test" };
  cli::ArgStream stream{ .argc = 1, .argv = argv };
  std::tuple<>   empty;

  EXPECT_THROW(my_cmd.parseStream(stream, empty), Exception);
}

TEST(CliTest, AppBuilderHelpAndVersion)
{
  bool executed = false;
  auto root_cmd = cli::cmd<"myapp", "App description">().setAction([&] { executed = true; });

  auto app = cli::app(root_cmd).version("2.1.0");

  // Test --version
  char  arg_app[]  = "myapp";
  char  arg_ver[]  = "--version";
  char *argv_ver[] = { arg_app, arg_ver };
  EXPECT_TRUE(app.parse(2, argv_ver));
  EXPECT_FALSE(executed);

  // Test --help
  char  arg_help[]  = "--help";
  char *argv_help[] = { arg_app, arg_help };
  EXPECT_TRUE(app.parse(2, argv_help));
  EXPECT_FALSE(executed);

  // Test normal execution
  char *argv_run[] = { arg_app };
  EXPECT_TRUE(app.parse(1, argv_run));
  EXPECT_TRUE(executed);
}

TEST(CliTest, VariadicPositionalArguments)
{
  std::vector<std::string> targets;
  bool                     executed = false;

  auto install_cmd = cli::cmd<"install", "Install packages">()
                       .positional<"targets">(targets, "Packages to install")
                       .setAction([&] { executed = true; });

  auto root_cmd = cli::cmd<"zc", "Package manager">().subcommand(install_cmd).requiresSubcommand();

  const char    *argv[] = { "zc", "install", "pkg1", "pkg2", "pkg3" };
  cli::ArgStream stream{ .argc = 5, .argv = argv };
  std::tuple<>   empty;

  root_cmd.parseStream(stream, empty);

  EXPECT_TRUE(executed);
  ASSERT_EQ(targets.size(), 3U);
  EXPECT_EQ(targets[0], "pkg1");
  EXPECT_EQ(targets[1], "pkg2");
  EXPECT_EQ(targets[2], "pkg3");
}

} // namespace zc::tests
