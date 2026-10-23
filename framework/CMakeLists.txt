add_library(zc-framework STATIC
  framework/cli/AppBuilder.hpp
  framework/cli/ArgStream.hpp
  framework/cli/CommandBuilder.hpp
  framework/cli/detail.hpp
  framework/cli/FixedString.hpp
  framework/cli/Option.hpp
  framework/cli/Positional.hpp

  framework/ui/DefaultUIStrategy.hpp
  framework/ui/Table.hpp
  framework/ui/TableCharset.hpp
  framework/ui/UI.hpp
  framework/ui/UIStrategy.hpp

  framework/util/concepts.hpp
  framework/util/errors.hpp
  framework/util/helpers.hpp
  framework/util/raw_mode.hpp
  framework/util/string.hpp
  framework/util/terminal.hpp
  framework/util/types.hpp

  framework/attributes.h
  framework/Exception.hpp
  framework/fs.hpp
  framework/network.hpp
  framework/shell.hpp
  framework/signals.hpp
  framework/Timer.hpp
  framework/Version.hpp

  src/ui/Table.cpp

  src/fs.cpp
  src/network.cpp
  src/string.cpp
)
target_include_directories(zc-framework PUBLIC .)
target_link_libraries(zc-framework PUBLIC nlohmann_json::nlohmann_json)
