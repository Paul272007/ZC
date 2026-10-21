#include "framework/fs.hpp"

#include <fstream>
#include <sstream>

#include "framework/Exception.hpp"
#include "framework/util/errors.hpp"

namespace zc::fs
{

std::expected<std::string, Exception> read(const stdfs::path &file)
{
  std::ifstream input(file);
  if (!input.is_open())
    return std::unexpected(Exception(FSError::ReadingError, "file {} couldn't be read", file));
  std::stringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

std::expected<void, Exception> write(const stdfs::path &file, const std::string &content)
{
  std::ofstream output(file);
  if (!output.is_open())
    return std::unexpected(Exception(FSError::WritingError, "file {} couldn't be written", file));
  output << content;
  return {};
}

std::expected<json, Exception> read_json(const stdfs::path &file)
{
  json parsed_json;

  if (!file_exists(file))
    return std::unexpected(Exception(FSError::FileNotFound, "JSON file {} not found", file));

  std::ifstream input(file);
  if (!input.is_open())
    return std::unexpected(Exception(FSError::ReadingError, "JSON file {} couldn't be read", file));

  try {
    input >> parsed_json;
  } catch (const json::parse_error &e) {
    return std::unexpected(
      Exception(FSError::ParsingError, "JSON file {} couldn't be parsed: {}", file, e.what())
    );
  }
  return parsed_json;
}

std::expected<void, Exception> write_json(const stdfs::path &file, const json &content)
{
  std::ofstream output(file);
  if (!output.is_open())
    return std::unexpected(Exception(FSError::WritingError, "JSON file {} couldn't be written", file));
  output << content.dump(2);
  output.close();
  return {};
}

void remove(const std::filesystem::path &p)
{
  if (stdfs::is_directory(p))
    stdfs::remove_all(p);
  else if (stdfs::exists(p) || stdfs::is_symlink(p))
    stdfs::remove(p);
}

bool dir_exists(const stdfs::path &p)
{
  return stdfs::is_directory(p);
}

bool file_exists(const stdfs::path &p)
{
  std::error_code ec;
  return stdfs::is_regular_file(p, ec);
}

} // namespace zc::fs
