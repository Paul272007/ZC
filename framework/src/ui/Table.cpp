#include "framework/ui/Table.hpp"

#include "framework/ui/Style.hpp"
#include "framework/ui/UI.hpp"

namespace zc
{

namespace
{

void drawNTimes(const u64 n, const std::string_view c)
{
  for (u64 i = 0; i < n; i++)
    ui.print("{}", c);
}

void drawNTimes(const u64 n, const char c)
{
  ui.print("{}", std::string(n, c));
}

void padding(const u64 length)
{
  drawNTimes(length, ' ');
}

} // namespace

std::vector<u64> Table::widths() const
{
  std::vector<u64> widths;

  // Get max number of columns
  size_t n_cols = 0;
  for (const auto &row : content_)
    n_cols = std::max(row.size(), n_cols);

  // Set maximum of each column to 0
  widths.assign(n_cols, 0);

  // Find maximum width for each column
  for (size_t col = 0; col < n_cols; col++) {
    size_t max_col_width = 0;

    for (size_t row = 0; row < content_.size(); row++)
      max_col_width = std::max(max_col_width, cell(row, col).size());

    widths[col] = max_col_width + 2;
  }

  return widths;
}

Thickness Table::colThick(const u64 col) const noexcept
{
  return (has_row_headers_ && col == 1) ? col_separator_thickness_ : col_thickness_;
}

void Table::drawLine(
  const std::vector<u64> &widths, const Thickness h_thick, std::string_view TableCharset::*left,
  std::string_view TableCharset::*mid, std::string_view TableCharset::*right
) const
{
  if (h_thick == Thickness::None || widths.empty())
    return;

  const auto horiz = box(h_thick, Thickness::None).horiz;
  ui.print("{}", box(h_thick, col_border_thickness_).*left);

  for (u64 i = 0; i < widths.size(); i++) {
    if (i > 0)
      ui.print("{}", box(h_thick, colThick(i)).*mid);
    drawNTimes(widths[i], horiz);
  }

  ui.println("{}", box(h_thick, col_border_thickness_).*right);
}

void Table::drawTopLine(const std::vector<u64> &widths) const
{
  drawLine(
    widths, row_border_thickness_, &TableCharset::top_left, &TableCharset::tee_top, &TableCharset::top_right
  );
}

void Table::drawSeparatorLine(const std::vector<u64> &widths) const
{
  drawLine(
    widths, row_separator_thickness_, &TableCharset::tee_left, &TableCharset::cross,
    &TableCharset::tee_right
  );
}

void Table::drawMiddleLine(const std::vector<u64> &widths) const
{
  drawLine(widths, row_thickness_, &TableCharset::tee_left, &TableCharset::cross, &TableCharset::tee_right);
}

void Table::drawBottomLine(const std::vector<u64> &widths) const
{
  drawLine(
    widths, row_border_thickness_, &TableCharset::bottom_left, &TableCharset::tee_bottom,
    &TableCharset::bottom_right
  );
}

void Table::drawRow(const u64 idx, const std::vector<u64> &widths, const bool is_header) const
{
  ui.print("{}", box(Thickness::None, col_border_thickness_).vert);
  for (u64 i = 0; i < widths.size(); i++) {
    if (i > 0)
      ui.print("{}", box(Thickness::None, colThick(i)).vert);

    const bool             cell_is_header = is_header || (i == 0 && has_row_headers_);
    const std::string_view text           = cell(idx, i);
    ui.print(cell_is_header ? header_style_ : content_style_, " {}", text);
    padding(widths[i] - text.size() - 1);
  }
  ui.println("{}", box(Thickness::None, col_border_thickness_).vert);
}

std::string_view Table::cell(const u64 row, const u64 col) const
{
  if (row < content_.size() && col < content_[row].size())
    return content_[row][col];
  return "";
}

void Table::setBorderThickness(const Thickness row_border_thickness, const Thickness col_border_thickness)
{
  row_border_thickness_ = row_border_thickness;
  col_border_thickness_ = col_border_thickness;
}

void Table::setSeparatorThickness(
  const Thickness row_separator_thickness, const Thickness col_separator_thickness
)
{
  row_separator_thickness_ = row_separator_thickness;
  col_separator_thickness_ = col_separator_thickness;
}

void Table::setThickness(const Thickness row_thickness, const Thickness col_thickness)
{
  row_thickness_ = row_thickness;
  col_thickness_ = col_thickness;
}

void Table::setHeaderStyle(Style style)
{
  header_style_ = style;
}

void Table::setContentStyle(Style style)
{
  content_style_ = style;
}

void Table::draw() const
{
  if (content_.empty())
    return;

  const std::vector max_widths = widths();
  drawTopLine(max_widths);

  u64 idx = 0;
  if (has_col_headers_) {
    drawRow(idx++, max_widths, true);
    if (idx < content_.size())
      drawSeparatorLine(max_widths);
  }

  if (idx < content_.size()) {
    drawRow(idx++, max_widths);
    while (idx < content_.size()) {
      drawMiddleLine(max_widths);
      drawRow(idx++, max_widths);
    }
  }

  drawBottomLine(max_widths);
}

} // namespace zc
