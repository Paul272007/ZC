#include "framework/Timer.hpp"

#include <gtest/gtest.h>
#include <thread>

namespace zc::tests
{

TEST(TimerTest, MeasureExecution)
{
  {
    Timer timer("test_timer");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    EXPECT_EQ(timer.name, "test_timer");
  }
}

} // namespace zc::tests
