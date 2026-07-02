#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "clang_utils.h"
#include "helpers.h"

class ClangUtilsTest : public ::testing::Test
{
protected:
  std::filesystem::path temp_file = "test_code.c";

  void SetUp() override
  {
    std::ofstream out(temp_file);
    out << "#include <stdio.h>\n";
    out << "#include \"my_lib.h\"\n";
    out << "#define MAX_SIZE 100\n";
    out << "typedef int my_int;\n";
    out << "enum Color { RED, GREEN, BLUE };\n";
    out << "struct Point { int x; int y; };\n";
    out << "union Data { int i; float f; };\n";
    out << "int global_var = 10;\n";
    out << "void my_function(int a) { }\n";
    out << "int main() { return 0; }\n"; // Should be ignored
    out.close();
  }

  void TearDown() override
  {
    if (std::filesystem::exists(temp_file))
      std::filesystem::remove(temp_file);
  }
};

TEST_F(ClangUtilsTest, ParseDeclarations)
{
  zc::Declarations decls = zc::parse_declarations(temp_file);

  // Check includes
  ASSERT_EQ(decls.includes.size(), 2);
  EXPECT_EQ(decls.includes[0], "#include <stdio.h>\n");
  EXPECT_EQ(decls.includes[1], "#include \"my_lib.h\"\n");

  // Check macros
  ASSERT_EQ(decls.macros.size(), 1);
  EXPECT_EQ(decls.macros[0], "MAX_SIZE 100");

  // Check typedefs
  ASSERT_EQ(decls.typedefs.size(), 1);
  EXPECT_EQ(decls.typedefs[0], "typedef int my_int");

  // Check enums
  ASSERT_EQ(decls.enums.size(), 1);
  EXPECT_EQ(decls.enums[0], "enum Color { RED, GREEN, BLUE }");

  // Check structs
  ASSERT_EQ(decls.structs.size(), 1);
  EXPECT_EQ(decls.structs[0], "struct Point { int x; int y; }");

  // Check unions
  ASSERT_EQ(decls.unions.size(), 1);
  EXPECT_EQ(decls.unions[0], "union Data { int i; float f; }");

  // Check globals
  ASSERT_EQ(decls.globals.size(), 1);
  EXPECT_EQ(decls.globals[0], "extern int global_var");

  // Check functions (main is ignored)
  ASSERT_EQ(decls.functions.size(), 1);
  EXPECT_EQ(decls.functions[0], "void my_function(int a)");
}

TEST_F(ClangUtilsTest, GetFileIncludes)
{
  std::map<std::string, zc::Pkg> pkgs;
  zc::Pkg                        p;
  p.name         = "my_lib";
  p.origin       = "main";
  p.versions     = { zc::Version("1.0.0") };
  pkgs["my_lib"] = p;

  auto deps = zc::get_file_includes(temp_file, pkgs);
  ASSERT_EQ(deps.size(), 1);
  EXPECT_EQ(deps[0].name, "my_lib");
  EXPECT_EQ(deps[0].origin, "main");
  EXPECT_EQ(deps[0].version.string(), "1.0.0");
}
