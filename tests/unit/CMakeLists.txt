add_executable(zc_test
  test_helpers.cpp
  test_version.cpp
  test_make_variable.cpp
  test_pkg_type.cpp
  test_language.cpp
  test_language_conf.cpp
  test_dependency.cpp
  test_local_target.cpp
  test_pkg.cpp
  test_pconf.cpp
  test_project_utils.cpp
  test_clang_utils.cpp
  test_exceptions.cpp
  test_main.cpp
)

target_link_libraries(zc_test PRIVATE
  zc_core
  gtest
)

include(GoogleTest)
gtest_discover_tests(zc_test)
