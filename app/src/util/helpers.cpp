#include "util/helpers.hpp"

namespace zc::app
{

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

ListType parse_type(const std::string_view type)
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
