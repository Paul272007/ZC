#pragma once

#include <string>
#include <unordered_map>

#include "framework/util/helpers.hpp"

namespace zc::engine
{

class Ctx;

[[nodiscard]] std::string render(const std::string &text, const Ctx &ctx);

std::expected<void, Exception>
render_to(const std::filesystem::path &i_file, const std::filesystem::path &o_file, const Ctx &ctx);

} // namespace zc::engine
