#include "core/util/helpers.hpp"

#include <string_view>
#include <thread>

#include "core/project/Language.hpp"

namespace zc::core
{

std::string_view default_compiler(Language language)
{
  switch (language) {
  case Language::C:
    return "cc";
  case Language::CXX:
    return "c++";
  case Language::ASM:
    return "nasm";
  default:
    return "";
  }
}

std::string_view default_std(Language language)
{
  switch (language) {
  case Language::C:
    return "c23";
  case Language::CXX:
    return "c++26";
  default:
    return "";
  }
}

std::vector<std::string> default_flags(Language language)
{
  switch (language) {
  case Language::C:
  case Language::CXX:
    return { "-Wall", "-Wextra" };
  case Language::ASM:
  default:
    return {};
  }
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
