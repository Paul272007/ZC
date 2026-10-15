#include "framework/signals.hpp"

#include <gtest/gtest.h>

namespace zc::tests
{

inline bool custom_handler_called = false;

void test_signal_action(i32)
{
  custom_handler_called = true;
}

TEST(SignalsTest, InterruptedAndReset)
{
  signals::reset();
  EXPECT_FALSE(signals::interrupted());

  signals::detail::interrupted_ = 1;
  EXPECT_TRUE(signals::interrupted());

  signals::reset();
  EXPECT_FALSE(signals::interrupted());
}

TEST(SignalsTest, SignalHandlerRegistrationAndTrigger)
{
  custom_handler_called = false;
  signals::reset();

  signals::on<SIGUSR1, test_signal_action>();
  std::raise(SIGUSR1);

  EXPECT_TRUE(signals::interrupted());
  EXPECT_TRUE(custom_handler_called);

  signals::reset();
}

} // namespace zc::tests
