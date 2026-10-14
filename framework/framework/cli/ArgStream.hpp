#pragma once

#include <cassert>

#include "framework/util/types.hpp"

namespace zc::cli
{

struct ArgStream {
  const int    argc{};
  const char **argv{};

  int pos = 1; // Skip executable name

  [[nodiscard]] bool hasNext() const { return pos < argc; }

  [[nodiscard]] std::string_view current() const
  {
    assert(pos < argc && "ArgStream::current() called past end of arguments");
    return argv[pos];
  }

  [[nodiscard]] std::string_view peek() const { return (pos + 1 < argc) ? argv[pos + 1] : ""; }

  void advance(const u16 n = 1) { pos += n; }
};

} // namespace zc::cli
