#include "Table.h"

#include <iostream>
#include <string>

#include "ui_utils.h"

namespace zc
{

using namespace std;

namespace
{

void padding(int length)
{
  if (length > 0)
    std::cout << std::string(length, ' ');
}

} // namespace

Table::Table() {}

Table::Table(bool has_row_headers, bool has_col_headers, std::vector<std::vector<std::string>> content)
  : content_(std::move(content)), row_headers_(has_row_headers), col_headers_(has_col_headers)
{
}

void Table::set_thickness(
  bool rowThickness, bool colThickness, bool rowSeparatorThickness, bool colSeparatorThickness,
  bool rowBorderThickness, bool colBorderThickness
)
{
  row_thickness_           = rowThickness;
  col_thickness_           = colThickness;
  row_separator_thickness_ = rowSeparatorThickness;
  col_separator_thickness_ = colSeparatorThickness;
  row_border_thickness_    = rowBorderThickness;
  col_border_thickness_    = colBorderThickness;
}

void Table::set_content(std::vector<std::vector<std::string>> content)
{
  content_ = std::move(content);
}

/**
 * @brief For each column, get the size of the widest/longest element
 */
const string &Table::get_cell(int r, int c) const
{
  static const string empty = "";
  if (r < static_cast<int>(content_.size()) && c < static_cast<int>(content_[r].size()))
    return content_[r][c];
  return empty;
}

void Table::widths()
{
  int n_cols = 0;
  for (const auto &row : content_)
    if (row.size() > static_cast<size_t>(n_cols))
      n_cols = row.size();

  max_widths_.assign(n_cols, 0);
  for (int j = 0; j < n_cols; j++)
  {
    int max_col = 0;
    for (int i = 0; i < static_cast<int>(content_.size()); i++)
    {
      const int length = get_cell(i, j).size();
      if (length > max_col)
        max_col = length;
    }
    max_widths_[j] = max_col + 2;
  }
}

void Table::chars()
{
  // Cols and rows
  chars_.row_ = (row_thickness_) ? DOUBLE_HORIZONTAL : LIGHT_HORIZONTAL;
  chars_.col_ = (col_thickness_) ? DOUBLE_VERTICAL : LIGHT_VERTICAL;
  // Handle crosses
  int isSepRowDouble = (col_headers_ && row_separator_thickness_);
  int isSepColDouble = (row_headers_ && col_separator_thickness_);
  chars_.sepCross_ = (isSepColDouble)
                     ? (isSepRowDouble ? DOUBLE_VERTICAL_HORIZONTAL : VERTICAL_DOUBLE_HORIZONTAL_SINGLE)
                     : (isSepRowDouble ? VERTICAL_SINGLE_HORIZONTAL_DOUBLE : LIGHT_VERTICAL_HORIZONTAL);
  if (row_separator_thickness_)
  {
    chars_.sepRow_ = DOUBLE_HORIZONTAL;
    chars_.sepSepCross_ =
      (col_separator_thickness_) ? DOUBLE_VERTICAL_HORIZONTAL : VERTICAL_SINGLE_HORIZONTAL_DOUBLE;
  }
  else
  {
    chars_.sepRow_ = LIGHT_HORIZONTAL;
    chars_.sepSepCross_ =
      (col_separator_thickness_) ? VERTICAL_DOUBLE_HORIZONTAL_SINGLE : LIGHT_VERTICAL_HORIZONTAL;
  }
  if (row_thickness_)
    chars_.cross_ = (col_thickness_) ? DOUBLE_VERTICAL_HORIZONTAL : VERTICAL_SINGLE_HORIZONTAL_DOUBLE;
  else
    chars_.cross_ = (col_thickness_) ? VERTICAL_DOUBLE_HORIZONTAL_SINGLE : LIGHT_VERTICAL_HORIZONTAL;
  chars_.sepCol_ = (col_separator_thickness_) ? DOUBLE_VERTICAL : LIGHT_VERTICAL;
  // Rest
  if (row_border_thickness_)
  {
    chars_.borderRow_ = DOUBLE_HORIZONTAL;
    if (col_border_thickness_)
    {
      chars_.topLeftCorner_     = DOUBLE_DOWN_RIGHT;
      chars_.topRightCorner_    = DOUBLE_DOWN_LEFT;
      chars_.bottomLeftCorner_  = DOUBLE_UP_RIGHT;
      chars_.bottomRightCorner_ = DOUBLE_UP_LEFT;
    }
    else
    {
      chars_.topLeftCorner_     = DOWN_SINGLE_RIGHT_DOUBLE;
      chars_.topRightCorner_    = DOWN_SINGLE_LEFT_DOUBLE;
      chars_.bottomLeftCorner_  = UP_SINGLE_RIGHT_DOUBLE;
      chars_.bottomRightCorner_ = UP_SINGLE_LEFT_DOUBLE;
    }
    chars_.topT_    = (col_thickness_) ? DOUBLE_DOWN_HORIZONTAL : DOWN_SINGLE_HORIZONTAL_DOUBLE;
    chars_.bottomT_ = (col_thickness_) ? DOUBLE_UP_HORIZONTAL : UP_SINGLE_HORIZONTAL_DOUBLE;
    chars_.topSepT_ =
      (col_separator_thickness_) ? DOUBLE_DOWN_HORIZONTAL : DOWN_SINGLE_HORIZONTAL_DOUBLE;
    chars_.bottomSepT_ = (col_separator_thickness_) ? DOUBLE_UP_HORIZONTAL : UP_SINGLE_HORIZONTAL_DOUBLE;
  }
  else // !rowBorderThickness
  {
    chars_.borderRow_ = LIGHT_HORIZONTAL;
    if (col_border_thickness_)
    {
      chars_.topRightCorner_    = DOWN_DOUBLE_LEFT_SINGLE;
      chars_.topLeftCorner_     = DOWN_DOUBLE_RIGHT_SINGLE;
      chars_.bottomLeftCorner_  = UP_DOUBLE_RIGHT_SINGLE;
      chars_.bottomRightCorner_ = UP_DOUBLE_LEFT_SINGLE;
    }
    else
    {
      chars_.topLeftCorner_     = LIGHT_DOWN_RIGHT;
      chars_.topRightCorner_    = LIGHT_DOWN_LEFT;
      chars_.bottomLeftCorner_  = LIGHT_UP_RIGHT;
      chars_.bottomRightCorner_ = LIGHT_UP_LEFT;
    }
    chars_.topT_    = (col_thickness_) ? DOWN_DOUBLE_HORIZONTAL_SINGLE : LIGHT_DOWN_HORIZONTAL;
    chars_.bottomT_ = (col_thickness_) ? UP_DOUBLE_HORIZONTAL_SINGLE : LIGHT_UP_HORIZONTAL;
    chars_.topSepT_ = (col_separator_thickness_) ? DOWN_DOUBLE_HORIZONTAL_SINGLE : LIGHT_DOWN_HORIZONTAL;
    chars_.bottomSepT_ = (col_separator_thickness_) ? UP_DOUBLE_HORIZONTAL_SINGLE : LIGHT_UP_HORIZONTAL;
  }

  if (col_border_thickness_)
  {
    chars_.borderCol_ = DOUBLE_VERTICAL;
    chars_.leftT_     = (row_thickness_) ? DOUBLE_VERTICAL_RIGHT : VERTICAL_DOUBLE_RIGHT_SINGLE;
    chars_.leftSepT_ = (row_separator_thickness_) ? DOUBLE_VERTICAL_RIGHT : VERTICAL_DOUBLE_RIGHT_SINGLE;
    chars_.rightT_   = (row_thickness_) ? DOUBLE_VERTICAL_LEFT : VERTICAL_DOUBLE_LEFT_SINGLE;
    chars_.rightSepT_ = (row_separator_thickness_) ? DOUBLE_VERTICAL_LEFT : VERTICAL_DOUBLE_LEFT_SINGLE;
  }
  else
  {
    chars_.borderCol_ = LIGHT_VERTICAL;
    chars_.leftT_     = (row_thickness_) ? VERTICAL_SINGLE_RIGHT_DOUBLE : LIGHT_VERTICAL_RIGHT;
    chars_.leftSepT_  = (row_separator_thickness_) ? VERTICAL_SINGLE_RIGHT_DOUBLE : LIGHT_VERTICAL_RIGHT;
    chars_.rightT_    = (row_thickness_) ? VERTICAL_SINGLE_LEFT_DOUBLE : LIGHT_VERTICAL_LEFT;
    chars_.rightSepT_ = (row_separator_thickness_) ? VERTICAL_SINGLE_LEFT_DOUBLE : LIGHT_VERTICAL_LEFT;
  }
}

static void drawLine(const int length, const string &line)
{
  for (int i = 0; i < length; i++)
    cout << line;
}

void Table::top_line() const
{
  cout << chars_.topLeftCorner_;
  drawLine(max_widths_[0], chars_.borderRow_);

  for (size_t i = 1; i < max_widths_.size(); i++)
  {
    // 1. Separator first
    if (row_headers_ && i == 1)
      cout << chars_.topSepT_;
    else
      cout << chars_.topT_;

    // 2. Then the horizontal line for this row
    drawLine(max_widths_[i], chars_.borderRow_);
  }
  cout << chars_.topRightCorner_ << '\n';
}

void Table::bottom_line() const
{
  cout << chars_.bottomLeftCorner_;
  drawLine(max_widths_[0], chars_.borderRow_);

  for (size_t i = 1; i < max_widths_.size(); i++)
  {
    if (row_headers_ && i == 1)
      cout << chars_.bottomSepT_;
    else
      cout << chars_.bottomT_;

    drawLine(max_widths_[i], chars_.borderRow_);
  }
  cout << chars_.bottomRightCorner_ << '\n';
}

void Table::column_header_separator() const
{
  cout << chars_.leftSepT_;
  drawLine(max_widths_[0], chars_.sepRow_);

  for (size_t i = 1; i < max_widths_.size(); i++)
  {
    if (row_headers_ && i == 1)
      cout << chars_.sepSepCross_;
    else
      cout << chars_.sepCross_;

    drawLine(max_widths_[i], chars_.sepRow_);
  }
  cout << chars_.rightSepT_ << '\n';
}

void Table::separator() const
{
  cout << chars_.leftT_;
  drawLine(max_widths_[0], chars_.row_);

  for (size_t i = 1; i < max_widths_.size(); i++)
  {
    if (row_headers_ && i == 1)
      cout << chars_.sepCross_;
    else
      cout << chars_.cross_;

    drawLine(max_widths_[i], chars_.row_);
  }
  cout << chars_.rightT_ << '\n';
}

void Table::middle_line()
{
  // Left border
  cout << chars_.borderCol_;

  // First column
  const string &cell0 = get_cell(current_line_, 0);
  padding(max_widths_[0] - cell0.size() - 1);
  cout << cell0 << " ";

  // Other columns
  for (size_t i = 1; i < max_widths_.size(); i++)
  {
    cout << chars_.col_; // Internal vertical separator
    const string &cell = get_cell(current_line_, i);
    padding(max_widths_[i] - cell.size() - 1);
    cout << cell << " ";
  }

  current_line_++;
  // Right border
  cout << chars_.borderCol_ << '\n';
}

int Table::size() const
{
  return content_.size();
}

void Table::draw()
{
  if (content_.empty())
    return;

  widths();
  if (max_widths_.empty())
    return;

  chars();

  top_line();

  if (col_headers_)
  {
    middle_line();
    column_header_separator();
  }

  // If there are still lines to draw
  int n_rows = content_.size();
  if (current_line_ < n_rows)
  {
    middle_line();
    while (current_line_ < n_rows)
    {
      separator();
      middle_line();
    }
  }

  bottom_line();
  // In case the table is being displayed again
  current_line_ = 0;
}

} // namespace zc
