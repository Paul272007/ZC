add_library(zc-framework STATIC
  framework/util/concepts.hpp
  framework/util/errors.hpp
  framework/util/types.hpp

  framework/Exception.hpp
  framework/fs.hpp
  framework/Version.hpp

  src/fs.cpp
)
target_include_directories(zc-framework PUBLIC .)
target_link_libraries(zc-framework PUBLIC nlohmann_json::nlohmann_json)
