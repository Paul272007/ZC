#include <iostream>
#include <string>

#include "interface.hh"
#include "objects/Table.hh"

using namespace std;

namespace
{
void padding(int length)
{
  for (int i = 0; i < length; i++)
  {
    printf(" ");
  }
}
} // namespace

Table::Table(
    const int n_rows, const int n_cols, bool hasRowHeaders, bool hasColHeaders,
    const std::vector<std::vector<std::string>> &content
)
    : n_cols_(n_cols), n_rows_(n_rows), current_line_(0), content_(content), hasRowHeaders_(hasRowHeaders),
      hasColHeaders_(hasColHeaders)
{
  // Allocate internal vectors' size before using them
  max_widths_.resize(n_cols, 0);
  sizes_.resize(n_rows, std::vector<int>(n_cols, 0));
}

void Table::setThickness(
    bool rowThickness, bool colThickness, bool rowSeparatorThickness, bool colSeparatorThickness,
    bool rowBorderThickness, bool colBorderThickness
)
{
  rowThickness_ = rowThickness;
  colThickness_ = colThickness;
  rowSeparatorThickness_ = rowSeparatorThickness;
  colSeparatorThickness_ = colSeparatorThickness;
  rowBorderThickness_ = rowBorderThickness;
  colBorderThickness_ = colBorderThickness;
}

void Table::setContent(const std::vector<std::vector<std::string>> &content)
{
  content_ = content;
}

/**
 * @brief For each column, get the size of the widest/longest element
 */
void Table::getWidths()
{
  for (int j = 0; j < n_cols_; j++)
  {
    int max_col = 0;
    for (int i = 0; i < n_rows_; i++)
    {
      // Check if the line does exist in content_
      if (i < static_cast<int>(content_.size()) && j < static_cast<int>(content_[i].size()))
      {
        const int length = content_[i][j].size();
        sizes_[i][j] = length;
        if (length > max_col)
          max_col = length;
      }
    }
    max_widths_[j] = max_col + 2;
  }
}

void Table::getChars()
{
  // Cols and rows
  chars_.row_ = (rowThickness_) ? DOUBLE_HORIZONTAL : LIGHT_HORIZONTAL;
  chars_.col_ = (colThickness_) ? DOUBLE_VERTICAL : LIGHT_VERTICAL;
  // Handle crosses
  int isSepRowDouble = (hasColHeaders_ && rowSeparatorThickness_);
  int isSepColDouble = (hasRowHeaders_ && colSeparatorThickness_);
  chars_.sepCross_ = (isSepColDouble)
                         ? (isSepRowDouble ? DOUBLE_VERTICAL_HORIZONTAL : VERTICAL_DOUBLE_HORIZONTAL_SINGLE)
                         : (isSepRowDouble ? VERTICAL_SINGLE_HORIZONTAL_DOUBLE : LIGHT_VERTICAL_HORIZONTAL);
  if (rowSeparatorThickness_)
  {
    chars_.sepRow_ = DOUBLE_HORIZONTAL;
    chars_.sepSepCross_ =
        (colSeparatorThickness_) ? DOUBLE_VERTICAL_HORIZONTAL : VERTICAL_SINGLE_HORIZONTAL_DOUBLE;
  }
  else
  {
    chars_.sepRow_ = LIGHT_HORIZONTAL;
    chars_.sepSepCross_ =
        (colSeparatorThickness_) ? VERTICAL_DOUBLE_HORIZONTAL_SINGLE : LIGHT_VERTICAL_HORIZONTAL;
  }
  if (rowThickness_)
  {
    chars_.cross_ = (colThickness_) ? DOUBLE_VERTICAL_HORIZONTAL : VERTICAL_SINGLE_HORIZONTAL_DOUBLE;
  }
  else
  {
    chars_.cross_ = (colThickness_) ? VERTICAL_DOUBLE_HORIZONTAL_SINGLE : LIGHT_VERTICAL_HORIZONTAL;
  }
  chars_.sepCol_ = (colSeparatorThickness_) ? DOUBLE_VERTICAL : LIGHT_VERTICAL;
  // Rest
  if (rowBorderThickness_)
  {
    chars_.borderRow_ = DOUBLE_HORIZONTAL;
    if (colBorderThickness_)
    {
      chars_.topLeftCorner_ = DOUBLE_DOWN_RIGHT;
      chars_.topRightCorner_ = DOUBLE_DOWN_LEFT;
      chars_.bottomLeftCorner_ = DOUBLE_UP_RIGHT;
      chars_.bottomRightCorner_ = DOUBLE_UP_LEFT;
    }
    else
    {
      chars_.topLeftCorner_ = DOWN_SINGLE_RIGHT_DOUBLE;
      chars_.topRightCorner_ = DOWN_SINGLE_LEFT_DOUBLE;
      chars_.bottomLeftCorner_ = UP_SINGLE_RIGHT_DOUBLE;
      chars_.bottomRightCorner_ = UP_SINGLE_LEFT_DOUBLE;
    }
    chars_.topT_ = (colThickness_) ? DOUBLE_DOWN_HORIZONTAL : DOWN_SINGLE_HORIZONTAL_DOUBLE;
    chars_.bottomT_ = (colThickness_) ? DOUBLE_UP_HORIZONTAL : UP_SINGLE_HORIZONTAL_DOUBLE;
    chars_.topSepT_ = (colSeparatorThickness_) ? DOUBLE_DOWN_HORIZONTAL : DOWN_SINGLE_HORIZONTAL_DOUBLE;
    chars_.bottomSepT_ = (colSeparatorThickness_) ? DOUBLE_UP_HORIZONTAL : UP_SINGLE_HORIZONTAL_DOUBLE;
  }
  else // !rowBorderThickness
  {
    chars_.borderRow_ = LIGHT_HORIZONTAL;
    if (colBorderThickness_)
    {
      chars_.topRightCorner_ = DOWN_DOUBLE_LEFT_SINGLE;
      chars_.topLeftCorner_ = DOWN_DOUBLE_RIGHT_SINGLE;
      chars_.bottomLeftCorner_ = UP_DOUBLE_RIGHT_SINGLE;
      chars_.bottomRightCorner_ = UP_DOUBLE_LEFT_SINGLE;
    }
    else
    {
      chars_.topLeftCorner_ = LIGHT_DOWN_RIGHT;
      chars_.topRightCorner_ = LIGHT_DOWN_LEFT;
      chars_.bottomLeftCorner_ = LIGHT_UP_RIGHT;
      chars_.bottomRightCorner_ = LIGHT_UP_LEFT;
    }
    chars_.topT_ = (colThickness_) ? DOWN_DOUBLE_HORIZONTAL_SINGLE : LIGHT_DOWN_HORIZONTAL;
    chars_.bottomT_ = (colThickness_) ? UP_DOUBLE_HORIZONTAL_SINGLE : LIGHT_UP_HORIZONTAL;
    chars_.topSepT_ = (colSeparatorThickness_) ? DOWN_DOUBLE_HORIZONTAL_SINGLE : LIGHT_DOWN_HORIZONTAL;
    chars_.bottomSepT_ = (colSeparatorThickness_) ? UP_DOUBLE_HORIZONTAL_SINGLE : LIGHT_UP_HORIZONTAL;
  }

  if (colBorderThickness_)
  {
    chars_.borderCol_ = DOUBLE_VERTICAL;
    chars_.leftT_ = (rowThickness_) ? DOUBLE_VERTICAL_RIGHT : VERTICAL_DOUBLE_RIGHT_SINGLE;
    chars_.leftSepT_ = (rowSeparatorThickness_) ? DOUBLE_VERTICAL_RIGHT : VERTICAL_DOUBLE_RIGHT_SINGLE;
    chars_.rightT_ = (rowThickness_) ? DOUBLE_VERTICAL_LEFT : VERTICAL_DOUBLE_LEFT_SINGLE;
    chars_.rightSepT_ = (rowSeparatorThickness_) ? DOUBLE_VERTICAL_LEFT : VERTICAL_DOUBLE_LEFT_SINGLE;
  }
  else
  {
    chars_.borderCol_ = LIGHT_VERTICAL;
    chars_.leftT_ = (rowThickness_) ? VERTICAL_SINGLE_RIGHT_DOUBLE : LIGHT_VERTICAL_RIGHT;
    chars_.leftSepT_ = (rowSeparatorThickness_) ? VERTICAL_SINGLE_RIGHT_DOUBLE : LIGHT_VERTICAL_RIGHT;
    chars_.rightT_ = (rowThickness_) ? VERTICAL_SINGLE_LEFT_DOUBLE : LIGHT_VERTICAL_LEFT;
    chars_.rightSepT_ = (rowSeparatorThickness_) ? VERTICAL_SINGLE_LEFT_DOUBLE : LIGHT_VERTICAL_LEFT;
  }
}

static void drawLine(const int length, const string &line)
{
  for (int i = 0; i < length; i++)
    cout << line;
}

void Table::topLine() const
{
  cout << chars_.topLeftCorner_;
  drawLine(max_widths_[0], chars_.borderRow_);

  for (int i = 1; i < n_cols_; i++)
  {
    // 1. Separator first
    if (hasRowHeaders_ && i == 1)
      cout << chars_.topSepT_;
    else
      cout << chars_.topT_;

    // 2. Then the horizontal line for this row
    drawLine(max_widths_[i], chars_.borderRow_);
  }
  cout << chars_.topRightCorner_ << '\n';
}

void Table::bottomLine() const
{
  cout << chars_.bottomLeftCorner_;
  drawLine(max_widths_[0], chars_.borderRow_);

  for (int i = 1; i < n_cols_; i++)
  {
    if (hasRowHeaders_ && i == 1)
      cout << chars_.bottomSepT_;
    else
      cout << chars_.bottomT_;

    drawLine(max_widths_[i], chars_.borderRow_);
  }
  cout << chars_.bottomRightCorner_ << '\n';
}

void Table::columnHeaderSeparator() const
{
  cout << chars_.leftSepT_;
  drawLine(max_widths_[0], chars_.sepRow_);

  for (int i = 1; i < n_cols_; i++)
  {
    if (hasRowHeaders_ && i == 1)
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

  for (int i = 1; i < n_cols_; i++)
  {
    if (hasRowHeaders_ && i == 1)
      cout << chars_.sepCross_;
    else
      cout << chars_.cross_;

    drawLine(max_widths_[i], chars_.row_);
  }
  cout << chars_.rightT_ << '\n';
}

void Table::middleLine()
{
  // Left border
  cout << chars_.borderCol_;

  // First column
  padding(max_widths_[0] - sizes_[current_line_][0] - 1);
  cout << (content_[current_line_][0].empty() ? "" : content_[current_line_][0]) << " ";

  // Other columns
  for (int i = 1; i < n_cols_; i++)
  {
    cout << chars_.col_; // Internal vertical separator
    padding(max_widths_[i] - sizes_[current_line_][i] - 1);
    cout << (content_[current_line_][i].empty() ? "" : content_[current_line_][i]) << " ";
  }

  current_line_++;
  // Right border
  cout << chars_.borderCol_ << '\n';
}

int Table::getSize() const
{
  return n_rows_;
}

void Table::draw()
{
  if (n_rows_ == 0 || n_cols_ == 0)
    return;

  getWidths();
  getChars();

  topLine();

  if (hasColHeaders_)
  {
    middleLine();
    columnHeaderSeparator();
  }

  // If there are still lines to draw
  if (current_line_ < n_rows_)
  {
    middleLine();
    while (current_line_ < n_rows_)
    {
      separator();
      middleLine();
    }
  }

  bottomLine();
  // In case the table is being displayed again
  current_line_ = 0;
}
