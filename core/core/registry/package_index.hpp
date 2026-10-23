#pragma once

#include <expected>

#include "RemotePkg.hpp"

namespace zc::core::index
{

[[nodiscard]] std::expected<RemotePkg, Exception> resolve(std::string_view name, Version version);

[[nodiscard]] std::expected<std::map<std::string, Version, std::less<>>, Exception> availablePkgs();

[[nodiscard]] std::expected<stdfs::path, Exception> fetchAndExtract(const RemotePkg &pkg);

} // namespace zc::core::index
