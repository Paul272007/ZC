#include <gtest/gtest.h>

#include "project/MakeVariable.h"

TEST(MakeVariableTest, BasicOperations)
{
  zc::MakeVariable mv("CXXFLAGS");

  mv.add("-Wall");
  mv.add("-O3");

  // add() escapes strings. "-Wall" -> "'-Wall'", "-O3" -> "'-O3'"
  EXPECT_EQ(mv.string(), "'-Wall' '-O3'");
  EXPECT_EQ(mv.make_declaration(), "CXXFLAGS = '-Wall' '-O3'\n");
}

TEST(MakeVariableTest, AddNoEsc)
{
  zc::MakeVariable mv("LDFLAGS");
  mv.add_no_esc("-L/usr/lib");
  EXPECT_EQ(mv.string(), "-L/usr/lib");
}

TEST(MakeVariableTest, AddMacro)
{
  zc::MakeVariable mv("DEFINES");
  mv.add_macro("DEBUG", "1");
  mv.add_macro("NDEBUG", "");

  EXPECT_EQ(mv.string(), "'-DDEBUG=1' '-DNDEBUG'");
}

TEST(MakeVariableTest, AddMakeVar)
{
  zc::MakeVariable mv("OBJS");
  mv.add_make_var("SRC_FILES");

  EXPECT_EQ(mv.string(), "$(SRC_FILES)");
}
