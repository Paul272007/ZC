cmake_minimum_required(VERSION 3.12)
project(ZC VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- DEPENDENCIES ---

# 1. Fetch JSON (Nlohmann)
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

# 2. Find LLVM/Clang
# We search for the header packages first
find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)
find_package(CURL REQUIRED)
find_package(LibArchive REQUIRED)
find_package(OpenSSL REQUIRED)

message(STATUS "LLVM found: ${LLVM_PACKAGE_VERSION}")

include_directories(SYSTEM external)
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${Clang_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# 3. Find specifically the library binary (libclang.so / .dylib)
# It is important because target_link_libraries needs the path to the .so
find_library(CLANG_LIB
    NAMES clang libclang clang-18 clang-17 clang-16 clang-15
    HINTS ${LLVM_LIBRARY_DIRS} ${Clang_LIBRARY_DIRS}
)

if(NOT CLANG_LIB)
  message(FATAL_ERROR "Could not find libclang shared library.")
endif()
message(STATUS "Found libclang: ${CLANG_LIB}")

# --- BUILD ---

# Option to build in debug mode (activated for dev mode)
option(ENABLE_DEBUG "Activate debug mode" ON)

if(ENABLE_DEBUG)
  add_compile_definitions(DEBUG_MODE)
endif()

file(GLOB_RECURSE SOURCES "src/*.cc")
file(GLOB_RECURSE HEADERS "include/*.hh")

add_executable(zc ${SOURCES} ${HEADERS})

# Include directories for the project
target_include_directories(zc PRIVATE include)

# Link
target_link_libraries(zc PRIVATE
  ${CLANG_LIB}
  nlohmann_json::nlohmann_json
  CURL::libcurl
  LibArchive::LibArchive
  OpenSSL::Crypto
  # Sometimes necessary under Linux for LLVM :
  pthread
  dl
  tinfo
)

# --- INSTALLATION RULES (for install and uninstall scripts) ---

# Installs binary in /usr/local/bin (by default)
install(TARGETS zc DESTINATION bin)

# We don't install the configuration here because the script already does it
