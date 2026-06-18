#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace zc
{

/**
 * @brief Contains all characters corresponding to the width chosen by the
 * user
 */
struct TableChars
{
  std::string cross_;
  std::string sepCross_;
  std::string sepSepCross_;
  std::string row_;
  std::string col_;
  std::string sepRow_;
  std::string sepCol_;
  std::string borderCol_;
  std::string borderRow_;
  std::string topT_;
  std::string topSepT_;
  std::string bottomSepT_;
  std::string leftT_;
  std::string rightT_;
  std::string leftSepT_;
  std::string rightSepT_;
  std::string bottomT_;
  std::string topLeftCorner_;
  std::string bottomLeftCorner_;
  std::string topRightCorner_;
  std::string bottomRightCorner_;
};

class Table
{
public:
  using Chars = struct TableChars;

  Table();

  /**
   * @brief Create a Table instance
   * @param hasRowHeaders Whether the table has row headers
   * @param hasColHeaders Whether the table has column headers
   * @param content The content of the table
   */
  Table(bool has_row_headers, bool has_col_headers, std::vector<std::vector<std::string>> content);

  /**
   * @brief Print out the Table
   */
  void draw();

  /**
   * @brief Set the content of the table
   */
  void set_content(std::vector<std::vector<std::string>> content);

  /**
   * @brief Set the thickness of each border of the table
   */
  void set_thickness(
      bool rowThickness, bool colThickness, bool rowSeparatorThickness, bool colSeparatorThickness,
      bool rowBorderThickness, bool colBorderThickness
  );

  /**
   * @brief Get the table's size
   * @return The table's size
   */
  [[nodiscard]] int size() const;

private:
  /**
   * @brief Get the width of each column of the Table
   */
  void widths();

  /**
   * @brief Get the characters of the table borders corresponding to the chosen
   * border thicknesses
   */
  void chars();

  /**
   * @brief Draw the Table's top line
   */
  void top_line() const;

  /**
   * @brief Draw a table's middle line with its content
   */
  void middle_line();

  /**
   * @brief Draw the Table's bottom line
   */
  void bottom_line() const;

  /**
   * @brief Draw the separator between the column headers and the content of the table
   */
  void column_header_separator() const;

  /**
   * @brief Draw the separator between each line
   */
  void separator() const;

  /**
   * @brief Safely get a cell's content, returning empty string if out of bounds
   */
  const std::string &get_cell(int r, int c) const;

  /**
   * @brief The current line being displayed
   */
  size_t current_line_ = 0;

  /**
   * @brief Characters of the table borders
   */
  Chars chars_;

  /**
   * @brief The longest width for each column
   */
  std::vector<int> max_widths_;

  /**
   * @brief The content of the table
   */
  std::vector<std::vector<std::string>> content_;

  bool row_headers_ = false;
  bool col_headers_ = false;
  bool row_thickness_ = false;
  bool col_thickness_ = false;
  bool row_border_thickness_ = false;
  bool col_border_thickness_ = false;
  bool row_separator_thickness_ = false;
  bool col_separator_thickness_ = false;
};

} // namespace zc
