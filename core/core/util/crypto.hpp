#pragma once

#include <expected>

#include "framework/Exception.hpp"
#include "framework/util/types.hpp"

namespace zc::core::crypto {
  std::expected<std::string, Exception> sha256(const stdfs::path &path); } // namespace zc::core::crypto
