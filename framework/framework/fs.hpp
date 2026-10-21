#pragma once

#include <expected>

#include "framework/util/types.hpp"

namespace zc
{

class Exception;

namespace fs
{

std::expected<std::string, Exception> read(const stdfs::path &file);

std::expected<void, Exception> write(const stdfs::path &file, const std::string &content);

std::expected<json, Exception> read_json(const stdfs::path &file);

std::expected<void, Exception> write_json(const stdfs::path &file, const json &content);

void remove(const stdfs::path &path);

bool dir_exists(const stdfs::path &path);

bool file_exists(const stdfs::path &path);

} // namespace fs

} // namespace zc

template<>
struct std::formatter<std::filesystem::path> : std::formatter<std::string_view> {
  auto format(const std::filesystem::path &path, auto &ctx) const
  {
    return std::format_to(ctx.out(), "'{}'", path.string());
  }
};
