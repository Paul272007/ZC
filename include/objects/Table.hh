#pragma once

#include <string>
#include <vector>

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

  /**
   * @brief Create a Table instance
   *
   * @param n_rows The table's number of rows
   * @param n_cols The table's number of columns
   * @param hasRowHeaders Whether the table has row headers
   * @param hasColHeaders Whether the table has column headers
   * @param content The content of the table
   */
  Table(
      int n_rows, int n_cols, bool hasRowHeaders, bool hasColHeaders,
      const std::vector<std::vector<std::string>> &content
  );

  /**
   * @brief Print out the Table
   */
  void draw();

  /**
   * @brief Set the content of the table
   */
  void setContent(const std::vector<std::vector<std::string>> &content);

  /**
   * @brief Set the thickness of each border of the table
   */
  void setThickness(
      bool rowThickness, bool colThickness, bool rowSeparatorThickness,
      bool colSeparatorThickness, bool rowBorderThickness,
      bool colBorderThickness
  );

  /**
   * @brief Get the table's size
   *
   * @return The table's size
   */
  [[nodiscard]] int getSize() const;

private:
  /**
   * @brief Get the width of each column of the Table
   */
  void getWidths();

  /**
   * @brief Get the characters of the table borders corresponding to the chosen
   * border thicknesses
   */
  void getChars();

  /**
   * @brief Draw the Table's top line
   */
  void topLine() const;

  /**
   * @brief Draw a table's middle line with its content
   */
  void middleLine();

  /**
   * @brief Draw the Table's bottom line
   */
  void bottomLine() const;

  /**
   * @brief Draw the separator between the column headers and the content of the
   * table
   */
  void columnHeaderSeparator() const;

  /**
   * @brief Draw the separator between each line
   */
  void separator() const;

  /**
   * @brief The number of columns in the table
   */
  int n_cols_;

  /**
   * @brief The number of rows in the table
   */
  int n_rows_;

  /**
   * @brief The current line that is being displayed
   */
  int current_line_;

  /**
   * @brief Characters of the table borders
   */
  Chars chars_;

  /**
   * @brief The longest width for each column
   */
  std::vector<int> max_widths_;

  /**
   * @brief The body of the table
   */
  std::vector<std::vector<std::string>> content_;

  /**
   * @brief The size of each element
   */
  std::vector<std::vector<int>> sizes_;

  bool hasRowHeaders_ = false;
  bool hasColHeaders_ = false;
  bool rowThickness_ = false;
  bool colThickness_ = false;
  bool rowBorderThickness_ = false;
  bool colBorderThickness_ = false;
  bool rowSeparatorThickness_ = false;
  bool colSeparatorThickness_ = false;
};
