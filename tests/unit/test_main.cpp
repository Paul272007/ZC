#include <gtest/gtest.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>

int main(int argc, char **argv) {
    std::filesystem::path test_home = std::filesystem::current_path() / "test_home";
    std::filesystem::create_directories(test_home / ".zc");
    setenv("HOME", test_home.c_str(), 1);
    
    // Create dummy registry.json for testing LocalTarget::parse
    std::ofstream out(test_home / ".zc" / "registry.json");
    out << R"({
      "packages": {
        "pkgA": {
          "type": "BIN",
          "target": "pkgA",
          "origin": "main",
          "default": "1.0.0",
          "versions": {"1.0.0": {}}
        },
        "pkgB": {
          "type": "BIN",
          "target": "pkgB",
          "origin": "main",
          "default": "2.0.0",
          "versions": {"latest": {}, "2.0.0": {}}
        }
      }
    })";
    out.close();

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
