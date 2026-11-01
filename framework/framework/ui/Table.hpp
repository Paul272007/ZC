#pragma once

#include <format>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "framework/ui/Style.hpp"
#include "framework/ui/TableCharset.hpp"
#include "framework/util/concepts.hpp"
#include "framework/util/types.hpp"

namespace zc
{

template<Formattable T = std::string>
using TableContent = std::vector<std::vector<T>>;

class Table
{
  bool has_row_headers_ : 1;
  bool has_col_headers_ : 1;

  Thickness row_border_thickness_    : 3 { Thickness::Single };
  Thickness col_border_thickness_    : 3 { Thickness::Single };
  Thickness row_separator_thickness_ : 3 { Thickness::Single };
  Thickness col_separator_thickness_ : 3 { Thickness::Single };
  Thickness row_thickness_           : 3 { Thickness::Single };
  Thickness col_thickness_           : 3 { Thickness::Single };

  Style header_style_  = Style::Bold;
  Style content_style_ = Style::None;

  TableContent<> content_;

  [[nodiscard]] std::vector<u64> widths() const;
  [[nodiscard]] Thickness colThick(u64 col) const noexcept;

  // ——— Draw utilities ———

  void drawLine(
    const std::vector<u64> &widths, Thickness h_thick, std::string_view TableCharset::*left,
    std::string_view TableCharset::*mid, std::string_view TableCharset::*right
  ) const;
  void drawTopLine(const std::vector<u64> &widths) const;
  void drawBottomLine(const std::vector<u64> &widths) const;
  void drawSeparatorLine(const std::vector<u64> &widths) const;
  void drawMiddleLine(const std::vector<u64> &widths) const;
  void drawRow(u64 idx, const std::vector<u64> &widths, bool is_header = false) const;

public:
  template<Formattable T = std::string>
  explicit Table(
    TableContent<T> content, const bool has_row_headers = false, const bool has_col_headers = false
  )
    : has_row_headers_(has_row_headers), has_col_headers_(has_col_headers)
  {
    if constexpr (std::is_same_v<T, std::string>) {
      content_ = std::move(content);
    } else {
      content_.reserve(content.size());
      for (const auto &row : content) {
        std::vector<std::string> str_row;
        str_row.reserve(row.size());
        for (const auto &item : row)
          str_row.push_back(std::format("{}", item));
        content_.push_back(std::move(str_row));
      }
    }
  }

  // ——— Getter ———

  [[nodiscard]] u64 size() const noexcept { return content_.size(); }

  [[nodiscard]] std::string_view cell(u64 row, u64 col) const;

  // ——— Setters ———

  void setBorderThickness(Thickness row_border_thickness, Thickness col_border_thickness);

  void setSeparatorThickness(Thickness row_separator_thickness, Thickness col_separator_thickness);

  void setThickness(Thickness row_thickness, Thickness col_thickness);

  void setHeaderStyle(Style style);

  void setContentStyle(Style style);

  // ——— Public interface ———

  void draw() const;
};

} // namespace zc
