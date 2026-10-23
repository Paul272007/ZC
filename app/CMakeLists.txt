add_executable(zc-app
  src/commands/BuildCommand.hpp
  src/commands/Command.hpp
  src/commands/InstallCommand.hpp
  src/commands/Install.hpp

  src/context/BuildCommandCtx.hpp
  src/context/CommandCtx.hpp
  src/context/InstallCommandCtx.hpp

  src/helpers.hpp
  src/helpers.cpp
  src/main.cpp
)
set_target_properties(zc-app PROPERTIES OUTPUT_NAME "zc")
target_include_directories(zc-app PRIVATE src)
target_link_libraries(zc-app PRIVATE
  zc-framework
  zc-engine
  zc-core
)
