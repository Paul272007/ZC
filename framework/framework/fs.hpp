#pragma once

#include "framework/util/types.hpp"

namespace zc::fs
{

std::string read(const stdfs::path &file);

void write(const stdfs::path &file, const std::string &content);

json read_json(const stdfs::path &file);

void write_json(const stdfs::path &file, const json &content);

void remove(const stdfs::path &path);

bool dir_exists(const stdfs::path &path);

bool file_exists(const stdfs::path &path);

} // namespace zc::fs

template<>
struct std::formatter<std::filesystem::path> : std::formatter<std::string_view> {
  static auto format(const std::filesystem::path &path, std::format_context &ctx)
  {
    return std::format_to(ctx.out(), "'{}'", path.string());
  }
};
