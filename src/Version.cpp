/**
 * Project ZC
 * @author Paul Maillard
 * @version 0.1
 */

#include <format>

#include "Version.h"
#include "excepts/ZCException.h"
#include "helpers.h"

ZC_DEV_CONFIG

namespace zc
{

/**
 * Version implementation
 */

/**
 * @param text
 */
Version::Version(const std::string &text)
{
  std::stringstream ss(text);
  std::vector<int> parts;

  try
  {
    std::string segment;
    // Get 3 first numbers in string
    while (std::getline(ss, segment, '.'))
      if (!segment.empty())
        parts.push_back(std::stoi(segment));
  }
  catch (const std::exception &e)
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

/**
 * @param major
 * @param minor
 * @param patch
 */
Version::Version(const int major, const int minor, const int patch)
    : major_(major), minor_(minor), patch_(patch)
{
}

std::string Version::string() const
{
  return std::format("{}.{}.{}", major_, minor_, patch_);
}
} // namespace zc
