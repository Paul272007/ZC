#pragma once

#include "framework/ui/UIStrategy.hpp"
#include "framework/util/terminal.hpp"

namespace zc
{

class DefaultUIStrategy : public UIStrategy
{
public:
  explicit DefaultUIStrategy(const UI &ui) : UIStrategy(ui) {}

  [[nodiscard]] std::string info(std::string_view msg) const override
  {
    return std::format("{}➜{} {}", cyan, reset, msg);
  }

  [[nodiscard]] std::string success(std::string_view msg) const override
  {
    return std::format("{}✔{} {}", green, reset, msg);
  }

  [[nodiscard]] std::string warning(std::string_view msg) const override
  {
    return std::format("{}!{} {}", yellow, reset, msg);
  }

  [[nodiscard]] std::string debug(std::string_view msg) const override
  {
    return std::format("{}⚙{} {}", purple, reset, msg);
  }

  [[nodiscard]] std::string error(std::string_view msg) const override
  {
    return std::format("{}✗{} {}", red, reset, msg);
  }

  [[nodiscard]] std::string exception(const Exception &e) const override
  {
    return std::format("{}✗ (code: {}) {}{} ", red, e.code(), reset, e.what());
  }

  [[nodiscard]] std::string prompt(std::string_view msg) const override
  {
    return std::format("{}? {}{} ", blue, reset, msg);
  }

  [[nodiscard]] std::string prompt(std::string_view msg, std::string_view default_ans) const override
  {
    return std::format("{}?{} {} ({}) ", blue, reset, msg, default_ans);
  }

  [[nodiscard]] std::string resolved_prompt(std::string_view msg, std::string_view ans) const override
  {
    return std::format("{}?{} {} {}{}{}", blue, reset, msg, blue, ans, reset);
  }

  [[nodiscard]] std::string bullet_list_el(std::string element, const ULStyle style) const override
  {
    switch (style) {
    case ULStyle::Dot:
      return std::format("• {}", element);
    case ULStyle::Hyphen:
      return std::format("- {}", element);
    case ULStyle::Dash:
    default:
      return std::format("— {}", element);
    }
  }

  [[nodiscard]] std::string
  ordered_list_el(std::string element, u32 idx, const OLStyle style) const override
  {
    switch (style) {
    case OLStyle::Paren:
      return std::format("{}) {}", idx, element);
    case OLStyle::Dash:
      return std::format("{}— {}", idx, element);
    case OLStyle::Dot:
    default:
      return std::format("{}. {}", idx, element);
    }
  }

  [[nodiscard]] std::string
  checkbox(std::string element, const bool selected, const bool hovered) const override
  {
    return std::format(
      "{}{} {} {}{}", hovered ? blue : white, hovered ? ">" : " ", selected ? "◉" : "◯", element, reset
    );
  }

  [[nodiscard]] std::string radio(std::string element, const bool hovered) const override
  {
    return std::format("{}{} {}{}", hovered ? blue : white, hovered ? ">" : " ", element, reset);
  }
};

} // namespace zc
