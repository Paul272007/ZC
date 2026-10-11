#pragma once

#include <csignal>

#include "framework/util/raw_mode.hpp"
#include "framework/util/terminal.hpp"
#include "framework/util/types.hpp"

namespace zc::signals
{

namespace detail { inline volatile std::sig_atomic_t interrupted_ = 0; } // namespace detail

using Action = void (*)(i32);

[[nodiscard]] inline bool interrupted() noexcept
{
  return detail::interrupted_ != 0;
}

/// @brief Reset interruption flag
inline void reset() noexcept
{
  detail::interrupted_ = 0;
}

/// @brief Set signal handler
/// @tparam Sig The number of the signal for which the action is defined
/// @tparam Action The action to perform when this signal is received
/// @tparam Propagate Whether to propagate the signal or not
template<i32 Sig, auto Action = nullptr, bool Propagate = false>
void on()
{
  struct sigaction sa{};

  sa.sa_handler = [](int s) {
    detail::interrupted_ = 1;

    set_raw_mode(false);
    write(STDERR_FILENO, show_cursor.data(), show_cursor.size());

    if constexpr (requires { Action(s); })
      Action(s);
    else if (requires { Action(); })
      Action();

    if constexpr (Propagate) {
      (void)std::signal(s, SIG_DFL);
      (void)std::raise(s);
    }
  };

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(Sig, &sa, nullptr);
}

} // namespace zc::signals
