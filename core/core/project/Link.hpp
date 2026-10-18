#pragma once

#include "framework/util/helpers.hpp"
#include "framework/util/types.hpp"

namespace zc::core
{

enum class Link : u8 {
  Static,
  Dynamic,
};

inline void to_json(json &j, const Link l)
{
  j = to_str(l).value_or("Dynamic");
}

inline void from_json(const json &j, Link &l)
{
  l = from_str<Link>(j).value_or(Link::Dynamic);
}

} // namespace zc::core
