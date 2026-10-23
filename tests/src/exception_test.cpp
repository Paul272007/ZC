#include "framework/Exception.hpp"

#include <gtest/gtest.h>

#include "framework/util/errors.hpp"

namespace zc::tests
{

TEST(ExceptionTest, ConstructionAndCode)
{
  Exception ex1(CliError::UnknownFlag);
  EXPECT_EQ(ex1.code(), exit_code(CliError::UnknownFlag));
  EXPECT_TRUE(ex1.is(CliError::UnknownFlag));
  EXPECT_EQ(ex1, CliError::UnknownFlag);
  EXPECT_NE(ex1, CliError::MissingFlag);
  EXPECT_NE(ex1, FSError::FileNotFound);
  EXPECT_STREQ(ex1.what(), "Error");

  Exception ex2(CliError::UnknownFlag, "Flag '{}' not recognized", "--foo");
  EXPECT_EQ(ex2.code(), exit_code(CliError::UnknownFlag));
  EXPECT_STREQ(ex2.what(), "Error: Flag '--foo' not recognized.");
}

TEST(ExceptionTest, FormatErrors)
{
  Exception ex(FormatError::InvalidVersion, "Invalid version: {}", "v1");
  EXPECT_EQ(
    ex.code(), exit_code_base(FormatError::InvalidVersion) + static_cast<i32>(FormatError::InvalidVersion)
  );
  EXPECT_STREQ(ex.what(), "Error: Invalid version: v1.");
}

TEST(ExceptionTest, FSErrors)
{
  Exception ex(FSError::FileNotFound, "File not found: {}", "/tmp/test.txt");
  EXPECT_EQ(ex.code(), exit_code_base(FSError::FileNotFound) + static_cast<i32>(FSError::FileNotFound));
  EXPECT_STREQ(ex.what(), "Error: File not found: /tmp/test.txt.");
}

TEST(ExceptionTest, Formatter)
{
  Exception   ex(CliError::MissingFlag, "Missing {}", "-f");
  std::string formatted = std::format("{}", ex);
  EXPECT_EQ(formatted, "Error: Missing -f.");
}

TEST(ExceptionTest, ThrowOrTerminate)
{
#ifdef ZC_EXCEPTIONS_ENABLED
  EXPECT_THROW(throw_or_terminate(Exception(CliError::UnknownFlag, "test throw")), Exception);
#endif
}

} // namespace zc::tests
