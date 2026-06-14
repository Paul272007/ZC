cmake_minimum_required(VERSION 3.20)
project(ZC VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- DEPENDENCIES ---

# Find LLVM/Clang : we search for the header packages first
find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)
find_package(CURL REQUIRED)
find_package(LibArchive REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)

# include_directories(SYSTEM external)
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${Clang_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Find specifically the library binary (libclang.so / .dylib)
find_library(CLANG_LIB
    NAMES clang libclang clang-18 clang-17 clang-16 clang-15
    HINTS ${LLVM_LIBRARY_DIRS} ${Clang_LIBRARY_DIRS}
)

if(NOT CLANG_LIB)
  message(FATAL_ERROR "Could not find libclang shared library.")
endif()

# --- BUILD ---

# Option to build in debug mode (activated for dev mode)
option(ENABLE_DEBUG "Activate debug mode" ON)

if(ENABLE_DEBUG)
  add_compile_definitions(DEBUG_MODE)
endif()

# Create library
add_library(zc_core STATIC
        src/commands/Command.h
        src/commands/Run.cpp
        src/commands/Run.h
        src/config/Conf.h
        src/config/Dependency.h
        src/config/GConf.cpp
        src/config/GConf.h
        src/config/PConf.cpp
        src/config/PConf.h
        src/excepts/ExitCode.h
        src/excepts/ZCException.cpp
        src/excepts/ZCException.h
        src/pkgs/Network.cpp
        src/pkgs/Network.h
        src/pkgs/Pkg.h
        src/pkgs/PkgType.h
        src/pkgs/Registry.cpp
        src/pkgs/Registry.h
        src/project/Project.cpp
        src/project/Project.h
        src/templates/TemplateEngine.cpp
        src/templates/TemplateEngine.h
        src/ui/Interface.cpp
        src/ui/Interface.h
        src/CLI11.h
        src/CompileMode.h
        src/helpers.cpp
        src/helpers.h
        src/Language.h
        src/Version.cpp
        src/Version.h
)

target_include_directories(zc_core PRIVATE src)
target_link_libraries(zc_core PRIVATE
  ${CLANG_LIB}
  nlohmann_json::nlohmann_json
  CURL::libcurl
  LibArchive::LibArchive
  OpenSSL::Crypto
)

# Compile zc
add_executable(zc src/zc.cpp)
target_link_libraries(zc PRIVATE zc_core)

# Compile zcc
add_executable(zcc src/zcc.cpp)
target_link_libraries(zcc PRIVATE zc_core)

# --- INSTALLATION RULES ---

install(TARGETS zc zcc DESTINATION bin)