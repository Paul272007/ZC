#include "core/project/Project.hpp"

#include <gtest/gtest.h>

#include "core/project/target/ProjectBinTarget.hpp"
#include "core/project/target/ProjectLibTarget.hpp"
#include "core/registry/Registry.hpp"
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

  // Check that build.ninja and compile_commands.json were generated
  EXPECT_TRUE(fs::file_exists(project.ninjaFile()));
  EXPECT_TRUE(fs::file_exists(temp_dir / "compile_commands.json"));
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

TEST(ProjectTest, GenerateBuildFileAndCompileLibrary)
{
  const stdfs::path temp_dir = stdfs::temp_directory_path() / "zc_test_project_lib";
  std::error_code   ec;
  stdfs::remove_all(temp_dir, ec);
  stdfs::create_directories(temp_dir / "include" / "zc_test_project_lib", ec);
  stdfs::create_directories(temp_dir / "src", ec);

  // Public header in include/<target_name>/math.hpp
  auto write_pub_res = fs::write(
    temp_dir / "include" / "zc_test_project_lib" / "math.hpp", "#pragma once\nint add(int a, int b);\n"
  );
  ASSERT_TRUE(write_pub_res.has_value());

  // Private header in src/internal.hpp
  auto write_priv_res =
    fs::write(temp_dir / "src" / "internal.hpp", "#pragma once\ninline int offset() { return 0; }\n");
  ASSERT_TRUE(write_priv_res.has_value());

  // Source file including both <zc_test_project_lib/math.hpp> and "internal.hpp"
  auto write_src_res = fs::write(
    temp_dir / "src" / "math.cpp",
    "#include <zc_test_project_lib/math.hpp>\n#include \"internal.hpp\"\nint add(int a, int b) { return a + b + offset(); }\n"
  );
  ASSERT_TRUE(write_src_res.has_value());

  const std::string zc_json = R"({
  "name": "sample_pkg",
  "author": "Paul",
  "description": "Test library target project",
  "version": "1.0.0",
  "type": "LIB",
  "output": "math"
})";

  auto write_zc_res = fs::write(temp_dir / "zc.json", zc_json);
  ASSERT_TRUE(write_zc_res.has_value());

  core::Project project(temp_dir);
  ASSERT_EQ(project.targets().size(), 1);

  auto build_res = project.buildAll(core::BuildMode::Debug);
  ASSERT_TRUE(build_res.has_value()) << build_res.error().what();

  auto *lib_target = dynamic_cast<core::ProjectLibTarget *>(project.targets().front().get());
  ASSERT_NE(lib_target, nullptr);
  ASSERT_EQ(lib_target->outputFiles().size(), 2);
  EXPECT_TRUE(fs::file_exists(lib_target->outputFiles().at(0)));
  EXPECT_TRUE(fs::file_exists(lib_target->outputFiles().at(1)));

  stdfs::remove_all(temp_dir, ec);
}

TEST(ProjectTest, MultiTargetWithInternalDependency)
{
  const stdfs::path temp_dir = stdfs::temp_directory_path() / "zc_test_multi_target";
  std::error_code   ec;
  stdfs::remove_all(temp_dir, ec);

  // Create lib target "mathlib"
  stdfs::create_directories(temp_dir / "mathlib" / "include" / "mathlib", ec);
  stdfs::create_directories(temp_dir / "mathlib" / "src", ec);
  ASSERT_TRUE(
    fs::write(
      temp_dir / "mathlib" / "include" / "mathlib" / "calc.hpp", "#pragma once\nint compute(int x);\n"
    )
      .has_value()
  );
  ASSERT_TRUE(
    fs::write(
      temp_dir / "mathlib" / "src" / "calc.cpp",
      "#include <mathlib/calc.hpp>\nint compute(int x) { return x * 2; }\n"
    )
      .has_value()
  );
  ASSERT_TRUE(
    fs::write(
      temp_dir / "mathlib" / "target.json",
      R"({
  "type": "LIB",
  "public_macros": { "MATH_BONUS": "7" }
})"
    )
      .has_value()
  );

  // Create bin target "app" depending on "mathlib"
  stdfs::create_directories(temp_dir / "app" / "src", ec);
  ASSERT_TRUE(
    fs::write(
      temp_dir / "app" / "src" / "main.cpp",
      "#include <mathlib/calc.hpp>\nint main() { return compute(10) + MATH_BONUS; }\n"
    )
      .has_value()
  );
  ASSERT_TRUE(
    fs::write(
      temp_dir / "app" / "target.json",
      R"({
  "type": "BIN",
  "internal_deps": { "mathlib": "Static" }
})"
    )
      .has_value()
  );

  // Root zc.json
  ASSERT_TRUE(
    fs::write(
      temp_dir / "zc.json",
      R"({
  "name": "multi_pkg",
  "author": "Paul",
  "description": "Multi target project",
  "version": "1.0.0",
  "targets": ["mathlib", "app"]
})"
    )
      .has_value()
  );

  core::Project project(temp_dir);
  ASSERT_EQ(project.targets().size(), 2);

  auto build_res = project.buildAll(core::BuildMode::Debug);
  ASSERT_TRUE(build_res.has_value()) << build_res.error().what();

  auto *bin_target = dynamic_cast<core::ProjectBinTarget *>(project.targets().at(1).get());
  ASSERT_NE(bin_target, nullptr);
  EXPECT_EQ(bin_target->execute({}, sh::Output::HideAll), 27);

  stdfs::remove_all(temp_dir, ec);
}

TEST(ProjectTest, RegistryInstallUpdateSymlinksAndExternalDep)
{
  const stdfs::path lib_dir = stdfs::temp_directory_path() / "zc_test_ext_lib";
  const stdfs::path app_dir = stdfs::temp_directory_path() / "zc_test_ext_app";
  std::error_code   ec;
  stdfs::remove_all(lib_dir, ec);
  stdfs::remove_all(app_dir, ec);
  (void)core::Registry::get().uninstall("zc_ext_pkg", true);

  // 1. Create external library package v1.0.0 (target name: "zc_test_ext_lib")
  stdfs::create_directories(lib_dir / "include" / "zc_test_ext_lib", ec);
  stdfs::create_directories(lib_dir / "src", ec);
  ASSERT_TRUE(
    fs::write(lib_dir / "include" / "zc_test_ext_lib" / "ext.hpp", "#pragma once\nint ext_val();\n")
      .has_value()
  );
  ASSERT_TRUE(
    fs::write(
      lib_dir / "src" / "ext.cpp", "#include <zc_test_ext_lib/ext.hpp>\nint ext_val() { return 11; }\n"
    )
      .has_value()
  );
  ASSERT_TRUE(
    fs::write(
      lib_dir / "zc.json",
      R"({
  "name": "zc_ext_pkg",
  "author": "Paul",
  "description": "External library package",
  "version": "1.0.0",
  "type": "LIB",
  "output": "ext"
})"
    )
      .has_value()
  );

  {
    core::Project lib_proj_v1(lib_dir);
    auto          inst_res = core::Registry::get().installFromPath(lib_proj_v1, 1, false);
    ASSERT_TRUE(inst_res.has_value()) << inst_res.error().what();
  }

  // Verify v1.0.0 symlinks exist
  const stdfs::path inc_symlink = core::include_links_dir() / "zc_ext_pkg" / "zc_test_ext_lib";
  EXPECT_TRUE(fs::symlink_exists(inc_symlink));

  // 2. Update package to v2.0.0 WITHOUT changing default version
  ASSERT_TRUE(
    fs::write(
      lib_dir / "src" / "ext.cpp", "#include <zc_test_ext_lib/ext.hpp>\nint ext_val() { return 22; }\n"
    )
      .has_value()
  );
  ASSERT_TRUE(
    fs::write(
      lib_dir / "zc.json",
      R"({
  "name": "zc_ext_pkg",
  "author": "Paul",
  "description": "External library package",
  "version": "2.0.0",
  "type": "LIB",
  "output": "ext"
})"
    )
      .has_value()
  );

  {
    core::Project lib_proj_v2(lib_dir);
    auto          upd_res = core::Registry::get().updateFromPath(lib_proj_v2, 1, false, false);
    ASSERT_TRUE(upd_res.has_value()) << upd_res.error().what();
  }

  const auto *pkg = core::Registry::get().getPkg("zc_ext_pkg");
  ASSERT_NE(pkg, nullptr);
  EXPECT_EQ(pkg->default_version, Version(1, 0, 0));

  // Switch default version to 2.0.0 and verify symlink still valid
  auto set_def_res = core::Registry::get().setDefaultVersion("zc_ext_pkg", Version(2, 0, 0));
  ASSERT_TRUE(set_def_res.has_value()) << set_def_res.error().what();
  EXPECT_EQ(pkg->default_version, Version(2, 0, 0));
  EXPECT_TRUE(fs::symlink_exists(inc_symlink));

  // 3. Build an app depending on zc_ext_pkg@2.0.0
  stdfs::create_directories(app_dir / "src", ec);
  ASSERT_TRUE(
    fs::write(
      app_dir / "src" / "main.cpp",
      "#include <zc_ext_pkg/zc_test_ext_lib/ext.hpp>\nint main() { return ext_val(); }\n"
    )
      .has_value()
  );
  ASSERT_TRUE(
    fs::write(
      app_dir / "zc.json",
      R"({
  "name": "ext_consumer",
  "author": "Paul",
  "description": "Consumer of external pkg",
  "version": "1.0.0",
  "type": "BIN",
  "dependencies": {
    "zc_ext_pkg": {
      "version": "2.0.0",
      "targets": [["zc_test_ext_lib", "Static"]]
    }
  }
})"
    )
      .has_value()
  );

  core::Project app_proj(app_dir);
  auto          build_res = app_proj.buildAll(core::BuildMode::Debug);
  ASSERT_TRUE(build_res.has_value()) << build_res.error().what();

  auto *bin_target = dynamic_cast<core::ProjectBinTarget *>(app_proj.targets().front().get());
  ASSERT_NE(bin_target, nullptr);
  EXPECT_EQ(bin_target->execute({}, sh::Output::HideAll), 22);

  // 4. Uninstall v2.0.0 -> default should fallback to v1.0.0
  auto uninst_v2 = core::Registry::get().uninstall("zc_ext_pkg", Version(2, 0, 0), true);
  ASSERT_TRUE(uninst_v2.has_value()) << uninst_v2.error().what();
  EXPECT_EQ(pkg->default_version, Version(1, 0, 0));
  EXPECT_TRUE(fs::symlink_exists(inc_symlink));

  // 5. Uninstall entire package -> symlinks should be removed
  auto uninst_all = core::Registry::get().uninstall("zc_ext_pkg", true);
  ASSERT_TRUE(uninst_all.has_value()) << uninst_all.error().what();
  EXPECT_FALSE(fs::symlink_exists(inc_symlink));

  stdfs::remove_all(lib_dir, ec);
  stdfs::remove_all(app_dir, ec);
}

} // namespace zc::tests
