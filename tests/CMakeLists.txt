FetchContent_MakeAvailable(googletest)

# Suppress compiler warnings for external GoogleTest
foreach(target gtest gmock gtest_main gmock_main)
  if (TARGET ${target})
    target_compile_options(${target} PRIVATE -w)
  endif()
endforeach()

add_executable(zc-tests
  src/main_test.cpp
)

target_link_libraries(zc-tests PRIVATE
  # Add other project components
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(zc-tests)
