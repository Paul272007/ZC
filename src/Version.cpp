#include "Version.h"

#include <format>

#include "excepts/ZCException.h"
#include "helpers.h"

ZC_DEV_CONFIG

namespace zc
{

Version::Version(const std::string &text)
{
  if (text == "latest")
  {
    (*this) = { 0, 0, 0 };
    return;
  }

  stringstream ss(text);
  vector<int>  parts;

  try
  {
    std::string segment;
    while (std::getline(ss, segment, '.')) // Get 3 first numbers in string
      if (!segment.empty())
        parts.push_back(std::stoi(segment));
  }
  catch (const std::exception &)
  {
    throw ZCException(ZCE_CONTENT_ERROR, "Invalid version format: " + text);
  }

  if (parts.size() >= 1)
    major_ = parts[0];
  if (parts.size() >= 2)
    minor_ = parts[1];
  if (parts.size() >= 3)
    patch_ = parts[2];
}

Version::Version(const int major, const int minor, const int patch)
  : major_(major), minor_(minor), patch_(patch)
{
}

std::string Version::string() const
{
  return std::format("{}.{}.{}", major_, minor_, patch_);
}

bool Version::empty() const
{
  return major_ == 0 && minor_ == 0 && patch_ == 0;
}

} // namespace zc
