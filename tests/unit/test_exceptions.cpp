#include <gtest/gtest.h>

#include "excepts/ExitCode.h"
#include "excepts/ZCException.h"

TEST(ExceptionsTest, ZCExceptionCreation)
{
  zc::ZCException e(zc::ZCE_NOT_FOUND, "File missing");

  EXPECT_EQ(e.code(), zc::ZCE_NOT_FOUND);
  EXPECT_STREQ(e.what(), "File missing");
}
