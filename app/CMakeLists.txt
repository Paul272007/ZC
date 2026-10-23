add_executable(zc-app
  src/commands/BuildCommand.hpp
  src/commands/Command.hpp
  src/commands/Install.hpp
  src/commands/InstallCommand.hpp
  src/commands/List.hpp
  src/commands/Uninstall.hpp
  src/commands/Update.hpp
  src/commands/Use.hpp

  src/context/BuildCommandCtx.hpp
  src/context/CommandCtx.hpp
  src/context/InstallCommandCtx.hpp

  src/util/helpers.hpp
  src/util/helpers.cpp
  src/util/ListType.hpp

  src/main.cpp
)
set_target_properties(zc-app PROPERTIES OUTPUT_NAME "zc")
target_include_directories(zc-app PRIVATE src)
target_link_libraries(zc-app PRIVATE
  zc-framework
  zc-engine
  zc-core
)
