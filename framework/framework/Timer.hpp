#pragma once

#include <chrono>
#include <string_view>

#include "ui/UI.hpp"

namespace zc
{

#ifdef ZC_DEBUG
  #define ZC_DEBUG_TIMER(name) zc::Timer _zc_timer(name);
#else
  #define ZC_DEBUG_TIMER(name)
#endif

struct Timer {
  const std::string_view                                   name;
  const std::chrono::time_point<std::chrono::steady_clock> start;

  explicit Timer(const std::string_view name) : name(name), start(std::chrono::steady_clock::now()) {}

  Timer(const Timer &)            = delete;
  Timer(Timer &&)                 = delete;
  Timer &operator=(const Timer &) = delete;
  Timer &operator=(Timer &&)      = delete;

  ~Timer()
  {
    const std::chrono::time_point             end{ std::chrono::steady_clock::now() };
    std::chrono::duration<double, std::milli> elapsed{ end - start };

    ui.debug("[{}] Duration: {} ms", name, elapsed.count());
  }
};

} // namespace zc
