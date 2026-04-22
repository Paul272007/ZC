#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <objects/Version.hh>

Version::Version(int major, int minor, int patch) : major_(major), minor_(minor), patch_(patch)
{
}

Version::Version(const std::string &v_str)
{
  std::stringstream ss(v_str);
  std::string segment;
  std::vector<int> parts;

  // Get 3 first numbers in string
  while (std::getline(ss, segment, '.'))
    parts.push_back(std::stoi(segment));

  if (parts.size() >= 1)
    major_ = parts[0];
  if (parts.size() >= 2)
    minor_ = parts[1];
  if (parts.size() >= 3)
    patch_ = parts[2];
}

std::tuple<int, int, int> Version::to_tuple() const
{
  return std::tie(major_, minor_, patch_);
}

std::string Version::string() const
{
  std::stringstream s;
  s << major_ << "." << minor_ << "." << patch_;
  return s.str();
}

Version Version::operator=(const std::string &s) const
{
  return Version(s);
}

std::ostream &operator<<(std::ostream &os, const Version &v)
{
  os << v.major_ << "." << v.minor_ << "." << v.patch_;
  return os;
}

bool Version::operator==(const Version &other) const
{
  return to_tuple() == other.to_tuple();
}

bool Version::operator!=(const Version &other) const
{
  return !(*this == other);
}

bool Version::operator<(const Version &other) const
{
  return to_tuple() < other.to_tuple();
}

bool Version::operator>(const Version &other) const
{
  return other < *this;
}

bool Version::operator<=(const Version &other) const
{
  return !(*this > other);
}

bool Version::operator>=(const Version &other) const
{
  return !(*this < other);
}
