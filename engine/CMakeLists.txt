add_library(zc-engine STATIC
  engine/Ctx.hpp
  engine/engine.hpp

  src/engine.cpp
)
target_include_directories(zc-engine PUBLIC .)
target_link_libraries(zc-engine PUBLIC
  zc-framework
)
