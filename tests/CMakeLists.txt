FetchContent_MakeAvailable(googletest)

# Suppress compiler warnings for external GoogleTest
foreach(target gtest gmock gtest_main gmock_main)
  if (TARGET ${target})
    target_compile_options(${target} PRIVATE -w)
  endif()
endforeach()

add_executable(zc-tests
  src/main_test.cpp
  src/util_string_test.cpp
  src/util_helpers_test.cpp
  src/exception_test.cpp
  src/version_test.cpp
  src/fs_test.cpp
  src/timer_test.cpp
  src/signals_test.cpp
  src/shell_test.cpp
  src/ui_test.cpp
  src/cli_test.cpp
)

target_link_libraries(zc-tests PRIVATE
  zc-framework
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(zc-tests)
