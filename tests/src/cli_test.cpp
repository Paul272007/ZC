#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "framework/cli/App.hpp"
#include "framework/cli/Args.hpp"
#include "framework/cli/ArgStream.hpp"
#include "framework/cli/CommandBase.hpp"
#include "framework/cli/detail.hpp"
#include "framework/cli/FixedString.hpp"
#include "framework/Exception.hpp"

namespace zc::tests
{

namespace
{

template<cli::Command Cmd>
bool parse_cmd(Cmd &cmd, const std::vector<const char *> &argv, const bool execute_action = false)
{
  cli::ArgStream stream{ .argc = static_cast<int>(argv.size()), .argv = argv.data() };
  return cmd.parseStream(stream, execute_action);
}

class SampleCmd final : public cli::CommandBase<"sample", "Sample command">
{
public:
  cli::ListPositional<std::string, "targets">   targets{ "Targets" };
  cli::Flag<"clean", 'c'>                       clean{ "Clean" };
  cli::Flag<"all", 'a'>                         all{ "All" };
  cli::Flag<"backup", 'b'>                      backup{ "Backup" };
  cli::Option<int, "jobs", 'j'>                 jobs{ 4, "Jobs" };
  cli::RequiredOption<std::string, "name", 'n'> name_opt{ "Name" };
  cli::ListOption<std::string, "run", 'r'>      run{ "Run" };

  bool executed = false;

  auto options() { return std::tie(targets, clean, all, backup, jobs, name_opt, run); }

  void execute() { executed = true; }
};

class PushCmd final : public cli::CommandBase<"push", "Push changes">
{
public:
  cli::RequiredPositional<std::string, "remote"> remote{ "Remote name" };
  cli::Positional<std::string, "branch">         branch{ "Branch name", "main" };
  cli::Flag<"force", 'f'>                        force{ "Force push" };

  bool executed = false;

  auto options() { return std::tie(remote, branch, force); }

  void execute() { executed = true; }
};

class StatusCmd final : public cli::CommandBase<"status", "Show status">
{
public:
  bool executed = false;

  void execute() { executed = true; }
};

using GitCli = cli::RootCommand<"git", "Version control", PushCmd, StatusCmd>;

class ReqListCmd final : public cli::CommandBase<"reqlist", "Required list test">
{
public:
  cli::RequiredListOption<std::string, "items", 'i'> items{ "Items" };

  auto options() { return std::tie(items); }

  void execute() {}
};

template<cli::FixedString Name, cli::FixedString Desc>
class SharedBaseCmd : public cli::CommandBase<Name, Desc>
{
public:
  cli::Flag<"quiet", 'q'> quiet{ "Quiet mode" };

  template<typename Self>
  auto allArgs(this Self &self)
  {
    auto &base = static_cast<SharedBaseCmd &>(self);
    if constexpr (requires { self.options(); })
      return std::tuple_cat(self.options(), std::tie(base.quiet));
    else
      return std::tie(base.quiet);
  }
};

class ChildWithSharedCmd final : public SharedBaseCmd<"child", "Child command">
{
public:
  cli::Flag<"verbose", 'v'> verbose{ "Verbose mode" };

  bool executed = false;

  auto options() { return std::tie(verbose); }

  void execute() { executed = true; }
};

} // namespace

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

TEST(CliTest, ArgDescriptorsAndMetadata)
{
  cli::Flag<"verbose", 'v'> f{ "Verbose output" };
  EXPECT_EQ(f.longName(), "verbose");
  EXPECT_EQ(f.shortName(), 'v');
  EXPECT_TRUE(f.hasShortName());
  EXPECT_FALSE(f.isRequired());
  EXPECT_TRUE(f.isSwitch());
  EXPECT_FALSE(f.isList());
  EXPECT_FALSE(f.isSet());
  EXPECT_FALSE(f);
  EXPECT_FALSE(*f);
  EXPECT_EQ(f.description(), "Verbose output");

  cli::Option<int, "jobs", 'j'> opt{ 4, "Number of jobs" };
  EXPECT_EQ(opt.longName(), "jobs");
  EXPECT_EQ(opt.shortName(), 'j');
  EXPECT_FALSE(opt.isRequired());
  EXPECT_FALSE(opt.isSwitch());
  EXPECT_FALSE(opt.isSet());
  EXPECT_FALSE(static_cast<bool>(opt));
  EXPECT_EQ(*opt, 4);

  cli::RequiredOption<std::string, "config"> req_opt{ "Config path" };
  EXPECT_TRUE(req_opt.isRequired());
  EXPECT_FALSE(req_opt.hasShortName());
  EXPECT_TRUE(req_opt->empty());

  cli::Positional<std::string, "file"> pos{ "Input file", "default.txt" };
  EXPECT_EQ(pos.name(), "file");
  EXPECT_FALSE(pos.isRequired());
  EXPECT_EQ(*pos, "default.txt");

  cli::RequiredPositional<std::string, "dest"> req_pos{ "Destination" };
  EXPECT_EQ(req_pos.name(), "dest");
  EXPECT_TRUE(req_pos.isRequired());
}

TEST(CliTest, DetailParseValue)
{
  int int_val = 0;
  EXPECT_TRUE(cli::detail::parse_value("123", int_val));
  EXPECT_EQ(int_val, 123);
  EXPECT_FALSE(cli::detail::parse_value("abc", int_val));
  EXPECT_FALSE(cli::detail::parse_value("123abc", int_val));

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

TEST(CliTest, CombinedShortFlags)
{
  SampleCmd cmd;
  EXPECT_TRUE(parse_cmd(cmd, { "sample", "-abc", "-n", "proj" }));

  EXPECT_TRUE(cmd.all);
  EXPECT_TRUE(cmd.backup);
  EXPECT_TRUE(cmd.clean);

  SampleCmd bad;
  try {
    (void)parse_cmd(bad, { "sample", "-abx", "-n", "proj" });
    FAIL() << "Expected UnknownFlag exception";
  } catch (const Exception &e) {
    EXPECT_EQ(e, CliError::UnknownFlag);
  }
}

TEST(CliTest, SelfDescribingCommandDefaults)
{
  SampleCmd cmd;
  EXPECT_TRUE(parse_cmd(cmd, { "sample", "--name", "x" }));

  EXPECT_FALSE(cmd.clean);
  EXPECT_FALSE(*cmd.clean);
  EXPECT_EQ(*cmd.jobs, 4);
  EXPECT_FALSE(cmd.jobs);
  EXPECT_EQ(*cmd.name_opt, "x");
  EXPECT_FALSE(cmd.run);
  EXPECT_TRUE(cmd.run->empty());
  EXPECT_TRUE(cmd.targets->empty());
}

TEST(CliTest, SelfDescribingCommandValuesAndSyntaxes)
{
  SampleCmd cmd;
  ASSERT_TRUE(parse_cmd(cmd, { "sample", "-c", "-j8", "--name=x", "a", "b", "-r", "p1", "p2" }, true));

  EXPECT_TRUE(cmd.clean);
  EXPECT_TRUE(cmd.jobs);
  EXPECT_EQ(*cmd.jobs, 8);
  EXPECT_EQ(*cmd.name_opt, "x");
  EXPECT_TRUE(cmd.run);
  ASSERT_EQ(cmd.run->size(), 2U);
  EXPECT_EQ((*cmd.run)[0], "p1");
  EXPECT_EQ((*cmd.run)[1], "p2");
  ASSERT_EQ(cmd.targets->size(), 2U);
  EXPECT_EQ((*cmd.targets)[0], "a");
  EXPECT_EQ((*cmd.targets)[1], "b");
  EXPECT_TRUE(cmd.executed);
}

TEST(CliTest, ListOptionVariants)
{
  // 1. Flag provided without arguments: `--run`
  {
    SampleCmd cmd;
    (void)parse_cmd(cmd, { "sample", "-n", "x", "--run" });
    EXPECT_TRUE(cmd.run);
    EXPECT_TRUE(cmd.run->empty());
  }

  // 2. Greedy list followed by another flag: `-r arg1 arg2 -c`
  {
    SampleCmd cmd;
    (void)parse_cmd(cmd, { "sample", "-n", "x", "-r", "arg1", "arg2", "-c" });
    EXPECT_TRUE(cmd.clean);
    EXPECT_TRUE(cmd.run);
    ASSERT_EQ(cmd.run->size(), 2U);
    EXPECT_EQ((*cmd.run)[0], "arg1");
    EXPECT_EQ((*cmd.run)[1], "arg2");
  }

  // 3. Comma-separated '=' syntax: `--run=foo,bar,baz`
  {
    SampleCmd cmd;
    (void)parse_cmd(cmd, { "sample", "-n", "x", "--run=foo,bar,baz" });
    EXPECT_TRUE(cmd.run);
    std::vector<std::string> collected(cmd.run.begin(), cmd.run.end());
    ASSERT_EQ(collected.size(), 3U);
    EXPECT_EQ(collected[0], "foo");
    EXPECT_EQ(collected[1], "bar");
    EXPECT_EQ(collected[2], "baz");
  }

  // 4. RequiredListOption fails when given 0 values
  {
    ReqListCmd cmd;
    try {
      (void)parse_cmd(cmd, { "reqlist", "--items" });
      FAIL() << "Expected MissingValue exception";
    } catch (const Exception &e) {
      EXPECT_EQ(e, CliError::MissingValue);
    }
  }
}

TEST(CliTest, DoubleDashEndOfOptions)
{
  SampleCmd cmd;
  (void)parse_cmd(cmd, { "sample", "-n", "x", "--", "-c", "--not-a-flag" });

  EXPECT_FALSE(cmd.clean);
  ASSERT_EQ(cmd.targets->size(), 2U);
  EXPECT_EQ((*cmd.targets)[0], "-c");
  EXPECT_EQ((*cmd.targets)[1], "--not-a-flag");
}

TEST(CliTest, ErrorDiagnostics)
{
  // Missing required option
  {
    SampleCmd cmd;
    try {
      (void)parse_cmd(cmd, { "sample", "-c" });
      FAIL() << "Expected MissingFlag exception";
    } catch (const Exception &e) {
      EXPECT_EQ(e, CliError::MissingFlag);
    }
  }

  // Missing option value
  {
    SampleCmd cmd;
    try {
      (void)parse_cmd(cmd, { "sample", "-n", "x", "--jobs" });
      FAIL() << "Expected MissingValue exception";
    } catch (const Exception &e) {
      EXPECT_EQ(e, CliError::MissingValue);
    }
  }

  // Invalid option value
  {
    SampleCmd cmd;
    try {
      (void)parse_cmd(cmd, { "sample", "-n", "x", "--jobs=notanint" });
      FAIL() << "Expected InvalidValue exception";
    } catch (const Exception &e) {
      EXPECT_EQ(e, CliError::InvalidValue);
    }
  }

  // Missing required positional
  {
    PushCmd cmd;
    try {
      (void)parse_cmd(cmd, { "push", "-f" });
      FAIL() << "Expected MissingPositional exception";
    } catch (const Exception &e) {
      EXPECT_EQ(e, CliError::MissingPositional);
    }
  }

  // Unexpected positional
  {
    StatusCmd cmd;
    try {
      (void)parse_cmd(cmd, { "status", "extra" });
      FAIL() << "Expected UnexpectedPositional exception";
    } catch (const Exception &e) {
      EXPECT_EQ(e, CliError::UnexpectedPositional);
    }
  }
}

TEST(CliTest, RootCommandAndApp)
{
  cli::App<GitCli> app("git 2.0.0");

  // --version returns false without executing a command
  {
    const char *argv[] = { "git", "--version" };
    EXPECT_FALSE(app.parse(2, argv));
  }

  // --help returns false without executing a command
  {
    const char *argv[] = { "git", "--help" };
    EXPECT_FALSE(app.parse(2, argv));
  }

  // Subcommand --help also returns false without executing
  {
    const char *argv[] = { "git", "push", "--help" };
    EXPECT_FALSE(app.parse(3, argv));
    EXPECT_FALSE(app.root().subcommand<PushCmd>().executed);
  }

  // Missing subcommand throws MissingSubCommand
  {
    const char *argv[] = { "git" };
    try {
      (void)app.parse(1, argv);
      FAIL() << "Expected MissingSubCommand exception";
    } catch (const Exception &e) {
      EXPECT_EQ(e, CliError::MissingSubCommand);
    }
  }

  // Unknown subcommand throws UnknownSubCommand
  {
    const char *argv[] = { "git", "rebase" };
    try {
      (void)app.parse(2, argv);
      FAIL() << "Expected UnknownSubCommand exception";
    } catch (const Exception &e) {
      EXPECT_EQ(e, CliError::UnknownSubCommand);
    }
  }

  // Subcommand dispatch and execution
  {
    const char *argv[] = { "git", "push", "origin", "develop", "-f" };
    app.run(5, argv);

    const auto &push = app.root().subcommand<PushCmd>();
    EXPECT_TRUE(push.executed);
    EXPECT_EQ(*push.remote, "origin");
    EXPECT_EQ(*push.branch, "develop");
    EXPECT_TRUE(push.force);
  }
}

TEST(CliTest, SharedIntermediateBaseAllArgs)
{
  ChildWithSharedCmd cmd;
  ASSERT_TRUE(parse_cmd(cmd, { "child", "-vq" }, true));

  EXPECT_TRUE(cmd.verbose);
  EXPECT_TRUE(cmd.quiet);
  EXPECT_TRUE(cmd.executed);
}

TEST(CliTest, DeclarationMacrosAndSnakeToKebab)
{
  class MacroCmd final : public cli::CommandBase<"macro", "Macro test">
  {
  public:
    ZC_REQUIRED_POSITIONAL(source_file, std::string_view, "Source file");
    ZC_POSITIONAL(out_dir, std::string_view, "Output dir", "dist");
    ZC_FLAG(dry_run, 'n', "Dry run");
    ZC_OPTION(max_jobs, int, 'j', "Max jobs", 2);
    ZC_REQUIRED_OPTION(api_key, std::string_view, 'k', "API key");
    ZC_LIST_OPTION(extra_args, std::string_view, 'e', "Extra args");

    auto options() { return std::tie(source_file_, out_dir_, dry_run_, max_jobs_, api_key_, extra_args_); }

    void execute() {}
  };

  MacroCmd cmd;
  EXPECT_EQ(cmd.source_file_.name(), "source-file");
  EXPECT_EQ(cmd.out_dir_.name(), "out-dir");
  EXPECT_EQ(*cmd.out_dir_, "dist");
  EXPECT_EQ(cmd.dry_run_.longName(), "dry-run");
  EXPECT_EQ(cmd.max_jobs_.longName(), "max-jobs");
  EXPECT_EQ(cmd.api_key_.longName(), "api-key");
  EXPECT_EQ(cmd.extra_args_.longName(), "extra-args");

  ASSERT_TRUE(parse_cmd(
    cmd, { "macro", "main.cpp", "--dry-run", "--max-jobs=6", "--api-key", "secret", "-e", "a", "b" }
  ));
  EXPECT_EQ(*cmd.source_file_, "main.cpp");
  EXPECT_EQ(*cmd.out_dir_, "dist");
  EXPECT_TRUE(cmd.dry_run_);
  EXPECT_EQ(*cmd.max_jobs_, 6);
  EXPECT_EQ(*cmd.api_key_, "secret");
  ASSERT_EQ(cmd.extra_args_->size(), 2U);
}

} // namespace zc::tests
