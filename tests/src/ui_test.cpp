#include "framework/ui/UI.hpp"

#include <gtest/gtest.h>

#include "framework/ui/DefaultUIStrategy.hpp"

namespace zc::tests
{

TEST(UITest, DefaultUIStrategyFormatting)
{
  DefaultUIStrategy strategy(ui);

  EXPECT_TRUE(strategy.info("info msg").contains("info msg"));
  EXPECT_TRUE(strategy.success("ok msg").contains("ok msg"));
  EXPECT_TRUE(strategy.warning("warn msg").contains("warn msg"));
  EXPECT_TRUE(strategy.debug("debug msg").contains("debug msg"));
  EXPECT_TRUE(strategy.error("err msg").contains("err msg"));

  Exception ex(CliError::UnknownFlag, "unknown option");
  EXPECT_TRUE(strategy.exception(ex).contains("unknown option"));

  EXPECT_TRUE(strategy.prompt("What is your name?").contains("What is your name?"));
  EXPECT_TRUE(strategy.prompt("Age?", "25").contains("25"));
  EXPECT_TRUE(strategy.resolved_prompt("Name", "Alice").contains("Alice"));

  EXPECT_EQ(strategy.bullet_list_el("item", ULStyle::Dot), "• item");
  EXPECT_EQ(strategy.bullet_list_el("item", ULStyle::Hyphen), "- item");
  EXPECT_EQ(strategy.bullet_list_el("item", ULStyle::Dash), "— item");

  EXPECT_EQ(strategy.ordered_list_el("item", 1, OLStyle::Dot), "1. item");
  EXPECT_EQ(strategy.ordered_list_el("item", 2, OLStyle::Paren), "2) item");
  EXPECT_EQ(strategy.ordered_list_el("item", 3, OLStyle::Dash), "3— item");

  EXPECT_TRUE(strategy.checkbox("opt", true, false).contains("◉"));
  EXPECT_TRUE(strategy.checkbox("opt", false, false).contains("◯"));
  EXPECT_TRUE(strategy.checkbox("opt", false, true).contains(">"));

  EXPECT_TRUE(strategy.radio("choice", true).contains(">"));
  EXPECT_FALSE(strategy.radio("choice", false).contains(">"));
}

class TestCustomStrategy : public DefaultUIStrategy
{
public:
  explicit TestCustomStrategy(const UI &ui_ref) : DefaultUIStrategy(ui_ref) {}

  [[nodiscard]] std::string info(std::string_view msg) const override
  {
    return std::format("[CUSTOM_INFO] {}", msg);
  }
};

TEST(UITest, UIModeAndCustomStrategy)
{
  UI::init(stdin, stdout, stderr, UIMode::Quiet);
  ui.info("Should not crash in quiet mode");

  UI::init(stdin, stdout, stderr, UIMode::Normal, std::make_unique<TestCustomStrategy>(UI::get()));
  ui.info("Hello custom");

  // Restore default
  UI::init();
  ui.info("Hello default");
}

TEST(UITest, StyleAnsiSequenceCombination)
{
  EXPECT_EQ(detail::ansi_seq(Style::None), "");
  EXPECT_EQ(detail::ansi_seq(Style::Bold), "\033[1m");
  EXPECT_EQ(detail::ansi_seq(Style::Red), "\033[31m");
  EXPECT_EQ(detail::ansi_seq(Style::Bold | Style::Underlined | Style::Red), "\033[1;4;31m");
  EXPECT_EQ(detail::ansi_seq(Style::SBlink | Style::Cyan), "\033[5;36m");
}

} // namespace zc::tests
