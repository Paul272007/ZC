#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

class Registry;

enum Language
{
  C,
  CPP,
  H,
  HPP,
  PY,
  PYC,
  ARCHIVE,
  DYN_LIB,
  OBJECT,
  INSTANCE,
  ASSEMBLER,
  MULTI_LANGUAGES,
  OTHER
};

std::string to_string(Language l);

using Declarations = std::map<std::string, std::vector<std::string>>;

class File
{
public:
  /**
   * @brief Create file instance
   *
   * @param path The path to the file
   */
  File(const std::string &path);

  /**
   * @brief Write content to the file
   *
   * @param content The content to be written
   * @return
   */
  void write(const std::string &content) const;

  /**
   * @brief Write C declarations to the file
   *
   * @param decls The declarations to be written
   * @return Whether the operation was successful
   */
  void writeDeclarations(const Declarations &decls) const;

  /**
   * @brief Get file content
   */
  [[nodiscard]] std::string read() const;

  /**
   * @brief Copy file's content into the instance calling the method
   *
   * @param file The file to be copied
   * @return Whether the operation was successful
   */
  void copy(const File &file) const;

  /**
   * @brief Check if file exists
   *
   * @return true if the file exists, false otherwise
   */
  [[nodiscard]] bool exists() const;

  /**
   * @brief Parse the file and extract all declarations (works for C only)
   */
  [[nodiscard]] std::unique_ptr<Declarations> parse() const;

  /**
   * @brief Get inclusions from file and return associated required libraries
   */
  [[nodiscard]] std::vector<std::string> getInclusions(const Registry &reg) const;

  /**
   * @brief Get file path
   */
  [[nodiscard]] std::filesystem::path getPath() const;

  /**
   * @brief Get the filename
   */
  [[nodiscard]] std::string getFilename() const;

  /**
   * @brief Get the file extension
   */
  [[nodiscard]] std::string getExt() const;

  /**
   * @brief Get the language of the file
   *
   * @return A value of the enum Language
   */
  [[nodiscard]] Language getLanguage() const;

  /**
   * @brief << operator overload
   *
   * @param stream The stream in which to write the filename
   * @param file The file to be written
   */
  friend std::ostream &operator<<(std::ostream &stream, const File &file);

private:
  std::filesystem::path path_;
  std::string filename_;
  Language language_;
};
