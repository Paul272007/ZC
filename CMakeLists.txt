cmake_minimum_required(VERSION 3.20)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

project(ZC VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- DEPENDENCIES ---

find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)
find_package(CURL REQUIRED)
find_package(LibArchive REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# --- BUILD ---

option(ENABLE_DEBUG "Activate debug mode" ON)
if(ENABLE_DEBUG)
  add_compile_definitions(DEBUG_MODE)
  add_compile_options(-fsanitize=address,undefined -g -O0)
  add_link_options(-fsanitize=address,undefined)
endif()

add_compile_options(
  -Wall
  -Wextra
  -Wpedantic

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

  # Remove comment warnings
  -Wno-comment
)

# Create core library for testing and executable
add_library(zc_core STATIC
        src/config/Conf.h
        src/config/Dependency.h
        src/config/GConf.cpp
        src/config/GConf.h
        src/config/PConf.cpp
        src/config/PConf.h
        src/config/Language.h

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
        src/project/MakeVariable.cpp
        src/project/MakeVariable.h

        src/templates/TemplateEngine.cpp
        src/templates/TemplateEngine.h

        src/ui/ShellCommand.cpp
        src/ui/ShellCommand.h
        src/ui/Interface.cpp
        src/ui/Interface.h
        src/ui/Table.cpp
        src/ui/Table.h
        src/ui/ui_utils.h

        src/clang_utils.cpp
        src/clang_utils.h
        src/CLI11.h
        src/CompileMode.h
        src/helpers.cpp
        src/helpers.h
        src/Version.cpp
        src/Version.h

        src/commands/Add.cpp
        src/commands/Add.h
        src/commands/Build.cpp
        src/commands/Build.h
        src/commands/Clean.cpp
        src/commands/Clean.h
        src/commands/Command.h
        src/commands/Config.cpp
        src/commands/Config.h
        src/commands/Create.cpp
        src/commands/Create.h
        src/commands/Init.cpp
        src/commands/Init.h
        src/commands/Install.cpp
        src/commands/Install.h
        src/commands/List.cpp
        src/commands/List.h
        src/commands/Login.cpp
        src/commands/Login.h
        src/commands/Logout.cpp
        src/commands/Logout.h
        src/commands/Publish.cpp
        src/commands/Publish.h
        src/commands/Remove.cpp
        src/commands/Remove.h
        src/commands/Run.cpp
        src/commands/Run.h
        src/commands/Setup.cpp
        src/commands/Setup.h
        src/commands/Uninstall.cpp
        src/commands/Uninstall.h
        src/commands/Update.cpp
        src/commands/Update.h
        src/commands/Use.cpp
        src/commands/Use.h
)

target_include_directories(zc_core PUBLIC src)
target_link_libraries(zc_core PUBLIC
        libclang
        nlohmann_json::nlohmann_json
        CURL::libcurl
        LibArchive::LibArchive
        OpenSSL::Crypto
)

# Main executable
add_executable(zc src/zc.cpp)
target_link_libraries(zc PRIVATE zc_core)

enable_testing()
add_subdirectory(tests/unit)

# --- INSTALLATION RULES ---

install(TARGETS zc DESTINATION bin)
