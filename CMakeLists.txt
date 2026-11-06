cmake_minimum_required(VERSION 3.31)
project(ZC VERSION 0.1 LANGUAGES C CXX)

# Options
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(CURL REQUIRED)
find_package(LibArchive REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)

# Tests
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# Build
option(ENABLE_DEBUG "Activate debug mode" ON)
if (ENABLE_DEBUG)
  add_compile_definitions(ZC_DEBUG)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g -O0)
  add_link_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  file(COPY ${CMAKE_SOURCE_DIR}/etc/build.ninja.template DESTINATION $ENV{HOME}/.zc-debug/templates)
else()
  add_compile_definitions(ZC_RELEASE)
  add_compile_definitions(NDEBUG)
endif()

add_compile_definitions(ZC_MAJOR=0)
add_compile_definitions(ZC_MINOR=1)
add_compile_definitions(ZC_PATCH=1)
add_compile_definitions(ZC_VERSION="0.1.1")

if (WIN32)
  add_compile_definitions(ZC_WINDOWS)
elseif (APPLE)
  add_compile_definitions(ZC_MACOS) # iOS too
elseif (UNIX)
  add_compile_definitions(ZC_LINUX) # BSD too
endif()

add_compile_options(
  -Wall
  -Wextra
  -Wpedantic
  -Werror

  -Wshadow
  -Wformat=2
  -Wnull-dereference
  -Wconversion
  -Wsign-conversion
  -Wdouble-promotion
  -Wuninitialized
  -Wunused-label
  # C++ only
  -Wnon-virtual-dtor
  -Wold-style-cast
  -Woverloaded-virtual
)

# Enable reflection for g++ with -std=c++26
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-freflection)
endif()

add_subdirectory(framework) # Framework to build CLIs in C++, used by ZC
add_subdirectory(engine)    # Template engine - depends on framework
add_subdirectory(core)      # ZC backend      - depends on zengine
add_subdirectory(app)       # ZC CLI frontend - depends on framework and backend

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
  enable_testing()
  add_subdirectory(tests)
endif()

# Installation
# install(TARGETS zc-frontend DESTINATION bin)
