#include "core/project/Project.hpp"

#include <gtest/gtest.h>

#include "core/project/target/ProjectBinTarget.hpp"
#include "framework/fs.hpp"

namespace zc::tests
{

TEST(ProjectTest, GenerateBuildFileAndCompileBinary)
{
  const stdfs::path temp_dir = stdfs::temp_directory_path() / "zc_test_project_bin";
  std::error_code   ec;
  stdfs::remove_all(temp_dir, ec);
  stdfs::create_directories(temp_dir / "src", ec);

  // Write a simple main.cpp
  const stdfs::path main_cpp      = temp_dir / "src" / "main.cpp";
  auto              write_src_res = fs::write(main_cpp, "int main() { return 42; }\n");
  ASSERT_TRUE(write_src_res.has_value());

  // Write zc.json
  const std::string zc_json = R"({
  "name": "sample_bin",
  "author": "Paul",
  "description": "Test binary target project",
  "version": "1.0.0",
  "type": "BIN",
  "output": "sample_bin"
})";

  auto write_zc_res = fs::write(temp_dir / "zc.json", zc_json);
  ASSERT_TRUE(write_zc_res.has_value());

  core::Project project(temp_dir);
  EXPECT_EQ(project.name(), "sample_bin");
  EXPECT_EQ(project.version(), Version(1, 0, 0));
  ASSERT_EQ(project.targets().size(), 1);

  // Generate build config
  auto gen_res = project.generateBuildConfig(core::BuildMode::Debug);
  ASSERT_TRUE(gen_res.has_value()) << gen_res.error().what();

  // Check that build.ninja was generated
  EXPECT_TRUE(fs::file_exists(project.ninjaFile()));
  auto ninja_content = fs::read(project.ninjaFile());
  ASSERT_TRUE(ninja_content.has_value());
  EXPECT_NE(ninja_content->find("rule compile_cxx"), std::string::npos);
  EXPECT_NE(ninja_content->find("rule link"), std::string::npos);
  EXPECT_NE(ninja_content->find("build all: phony"), std::string::npos);

  // Build the target
  auto build_res = project.buildAll(core::BuildMode::Debug);
  ASSERT_TRUE(build_res.has_value()) << build_res.error().what();

  // Execute the binary and check return code
  auto *bin_target = dynamic_cast<core::ProjectBinTarget *>(project.targets().front().get());
  ASSERT_NE(bin_target, nullptr);
  EXPECT_FALSE(bin_target->outputFiles().empty());
  EXPECT_TRUE(fs::file_exists(bin_target->outputFiles().front()));

  int exit_code = bin_target->execute({}, sh::Output::HideAll);
  EXPECT_EQ(exit_code, 42);

  // Clean up
  stdfs::remove_all(temp_dir, ec);
}

} // namespace zc::tests
