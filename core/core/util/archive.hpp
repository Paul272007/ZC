#pragma once

#include <expected>

#include "framework/Exception.hpp"
#include "framework/util/types.hpp"

namespace zc::core::archive
{

std::expected<void, Exception> extract(const stdfs::path &archive, const stdfs::path &dest);

} // namespace zc::core::archive
