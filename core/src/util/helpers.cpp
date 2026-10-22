#include "core/util/helpers.hpp"

#include <thread>

namespace zc::core
{

std::string default_compiler(Language language)
{
  switch (language) {
  case Language::C:
    return "clang";
  case Language::CXX:
    return "clang++";
  default:
    return "";
  }
}

std::string default_std(Language language)
{
  switch (language) {
  case Language::C:
    return "c17";
  case Language::CXX:
    return "c++26";
  default:
    return "";
  }
}

std::vector<std::string> default_flags(Language /*language*/)
{
  return {};
}

u32 default_jobs_count()
{
  const unsigned int c = std::thread::hardware_concurrency();
  return c == 0 ? 1 : c;
}

Language linker(const SourceMap &sources)
{
  if (sources.contains(Language::CXX) && !sources.at(Language::CXX).empty())
    return Language::CXX;
  return Language::C;
}

} // namespace zc::core
