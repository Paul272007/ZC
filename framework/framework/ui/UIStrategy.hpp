#pragma once

#include "framework/Exception.hpp"
#include "framework/util/types.hpp"

namespace zc
{

class UI;

enum class ULStyle : u8 {
  Dot,    // •
  Dash,   // —
  Hyphen, // -
};

enum class OLStyle : u8 {
  Dot,   // 1.
  Paren, // 1)
  Dash,  // 1 —
};

class UIStrategy
{
protected:
  const UI &ui_;

  explicit UIStrategy(const UI &ui) : ui_(ui) {}

public:
  UIStrategy(const UIStrategy &)            = delete;
  UIStrategy(UIStrategy &&)                 = delete;
  UIStrategy &operator=(const UIStrategy &) = delete;
  UIStrategy &operator=(UIStrategy &&)      = delete;
  virtual ~UIStrategy()                     = default;

  // ——— Simple messages

  [[nodiscard]] virtual std::string info(std::string_view msg) const = 0;

  [[nodiscard]] virtual std::string success(std::string_view msg) const = 0;

  [[nodiscard]] virtual std::string warning(std::string_view msg) const = 0;

  [[nodiscard]] virtual std::string debug(std::string_view msg) const = 0;

  /// @brief Format an error message
  /// @param msg The error message
  /// @return The formatted error message
  [[nodiscard]] virtual std::string error(std::string_view msg) const = 0;

  /// @brief Format an exception
  /// @param e The exception to format
  /// @return The formatted exception
  [[nodiscard]] virtual std::string exception(const Exception &e) const = 0;

  /// @brief Format a question for the user to answer
  /// @param msg The question to format
  /// @return The formatted question
  [[nodiscard]] virtual std::string prompt(std::string_view msg) const = 0;

  /// @brief Format a question with a default answer
  /// @param msg The question to format
  /// @param default_ans The default answer
  /// @return The formatted question
  [[nodiscard]] virtual std::string prompt(std::string_view msg, std::string_view default_ans) const = 0;

  [[nodiscard]] virtual std::string resolved_prompt(std::string_view msg, std::string_view ans) const = 0;

  [[nodiscard]] virtual std::string bullet_list_el(std::string element, ULStyle style) const = 0;

  [[nodiscard]] virtual std::string ordered_list_el(std::string element, u32 idx, OLStyle style) const = 0;

  [[nodiscard]] virtual std::string checkbox(std::string element, bool selected, bool hovered) const = 0;

  [[nodiscard]] virtual std::string radio(std::string element, bool hovered) const = 0;
};

} // namespace zc
