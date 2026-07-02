#include <gtest/gtest.h>

#include "config/Language.h"

TEST(LanguageTest, FromString)
{
  EXPECT_EQ(zc::language_from_str("c"), zc::Language::C);
  EXPECT_EQ(zc::language_from_str("cpp"), zc::Language::CXX);
  EXPECT_EQ(zc::language_from_str("cxx"), zc::Language::CXX);
  EXPECT_EQ(zc::language_from_str("c++"), zc::Language::CXX);
  EXPECT_EQ(zc::language_from_str("h"), zc::Language::H);
  EXPECT_EQ(zc::language_from_str("hpp"), zc::Language::HXX);
  EXPECT_EQ(zc::language_from_str("asm"), zc::Language::ASM_NASM);
  EXPECT_EQ(zc::language_from_str("sh"), zc::Language::SH);
  EXPECT_EQ(zc::language_from_str("unknown"), zc::Language::UNKNOWN_LANGUAGE);
}

TEST(LanguageTest, ToString)
{
  EXPECT_EQ(zc::language_to_str(zc::Language::C), "C");
  EXPECT_EQ(zc::language_to_str(zc::Language::CXX), "CXX");
  EXPECT_EQ(zc::language_to_str(zc::Language::HXX), "HXX");
  EXPECT_EQ(zc::language_to_str(zc::Language::UNKNOWN_LANGUAGE), "UNKNOWN_LANGUAGE");
}

TEST(LanguageTest, IsOfLanguage)
{
  EXPECT_TRUE(zc::is_of_language(zc::Language::C, "file.c"));
  EXPECT_TRUE(zc::is_of_language(zc::Language::CXX, "file.cpp"));
  EXPECT_FALSE(zc::is_of_language(zc::Language::C, "file.cpp"));
  EXPECT_FALSE(zc::is_of_language(zc::Language::CXX, "file"));
}

TEST(LanguageTest, LanguageOf)
{
  EXPECT_EQ(zc::language_of("file.c"), zc::Language::C);
  EXPECT_EQ(zc::language_of("file.cpp"), zc::Language::CXX);
  EXPECT_EQ(zc::language_of("file.unknown"), zc::Language::UNKNOWN_LANGUAGE);
  EXPECT_EQ(zc::language_of("file"), zc::Language::UNKNOWN_LANGUAGE);
}
