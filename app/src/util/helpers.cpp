#include "util/helpers.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "framework/Exception.hpp"
#include "framework/util/errors.hpp"

namespace zc::app
{

DepSpec parse_dep_spec(std::string_view raw)
{
  std::string_view target;
  if (const auto slash = raw.find('/'); slash != std::string_view::npos) {
    target = raw.substr(slash + 1);
    if (target.empty() || target.contains('/') || target.contains('@'))
      throw_or_terminate(Exception(
        CliError::InvalidValue, "invalid package format '{}': expected pkg[@version][/target]", raw
      ));
    raw = raw.substr(0, slash);
  }

  Version version;
  if (const auto at = raw.find('@'); at != std::string_view::npos) {
    const auto ver_str = raw.substr(at + 1);
    if (ver_str.empty())
      throw_or_terminate(Exception(CliError::InvalidValue, "missing version after '@' in '{}'", raw));
    version = ver_str;
    raw     = raw.substr(0, at);
  }

  if (raw.empty())
    throw_or_terminate(Exception(CliError::InvalidValue, "missing package name"));

  return { .package = raw, .version = version, .target = target };
}

std::map<std::string, Version, std::less<>> parse_targets(std::vector<std::string> &targets)
{
  std::map<std::string, Version, std::less<>> pairs;
  for (auto &target : targets)
    if (const size_t at_pos = target.find('@'); at_pos != std::string::npos)
      pairs.insert_or_assign(target.substr(0, at_pos), target.substr(at_pos + 1));
    else
      pairs.insert_or_assign(std::move(target), Version::latest());
  return pairs;
}

ListType parse_list_type(const std::string_view type)
{
  if (type.empty() || type == "p" || type == "packages")
    return ListType::Packages;
  if (type == "d" || type == "deps" || type == "dependencies")
    return ListType::Dependencies;
  if (type == "ft" || type == "templates" || type == "file-templates")
    return ListType::FileTemplates;
  if (type == "pt" || type == "project-templates")
    return ListType::ProjectTemplates;
  if (type == "r" || type == "remote")
    return ListType::Remote;
  return ListType::Unknown;
}

} // namespace zc::app
