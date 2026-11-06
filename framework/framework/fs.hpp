#pragma once

#include <expected>
#include <format>
#include <string>

#include "framework/attributes.h"
#include "framework/util/types.hpp"

namespace zc
{

class Exception;

namespace fs
{

ZC_NODISCARD std::expected<std::string, Exception> read(const stdfs::path &file);

std::expected<void, Exception> write(const stdfs::path &file, const std::string &content);

ZC_NODISCARD std::expected<json, Exception> read_json(const stdfs::path &file);

std::expected<void, Exception> write_json(const stdfs::path &file, const json &content);

bool remove(const stdfs::path &path);

bool dir_exists(const stdfs::path &path);

bool file_exists(const stdfs::path &path);

bool symlink_exists(const stdfs::path &path);

} // namespace fs

} // namespace zc

template<>
struct std::formatter<std::filesystem::path> : std::formatter<std::string_view> {
  auto format(const std::filesystem::path &path, auto &ctx) const
  {
    return std::format_to(ctx.out(), "'{}'", path.string());
  }
};
