cmake_minimum_required(VERSION 3.20)
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

# --- BUILD ---

option(ENABLE_DEBUG "Activate debug mode" ON)
if(ENABLE_DEBUG)
  add_compile_definitions(DEBUG_MODE)
endif()

# Create library
add_library(zc-core STATIC
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
        src/project/MakeVariable.cpp
        src/project/MakeVariable.h

        src/templates/TemplateEngine.cpp
        src/templates/TemplateEngine.h

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
        src/Language.h
        src/Version.cpp
        src/Version.h
        src/ZCCompiler.cpp
        src/ZCCompiler.h

        src/commands/Add.cpp
        src/commands/Add.h
        src/commands/Build.cpp
        src/commands/Build.h
        src/commands/Clean.cpp
        src/commands/Clean.h
        src/commands/Command.h
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

target_include_directories(zc-core PRIVATE src)
target_link_libraries(zc-core PRIVATE
        libclang
        nlohmann_json::nlohmann_json
        CURL::libcurl
        LibArchive::LibArchive
        OpenSSL::Crypto
)

# Compile zc
add_executable(zc src/zc.cpp)
target_link_libraries(zc PRIVATE zc-core)

# Compile zcc
add_executable(zcc src/zcc.cpp)
target_link_libraries(zcc PRIVATE zc-core)

# --- INSTALLATION RULES ---

install(TARGETS zc zcc DESTINATION bin)
